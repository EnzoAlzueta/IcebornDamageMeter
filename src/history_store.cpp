#include "runtime.hpp"
#include "history.hpp"
#include "hunt_archive.hpp"
#include "quests.hpp"
#include <fstream>
#include <filesystem>
#include <cstdio>
namespace meter {
namespace {
std::mutex storeMutex;
HANDLE storeEvent=nullptr;
History loadedHistory, pendingHistory;
HuntRecord pendingLast;
constexpr size_t ArchiveQueueCapacity=32;
std::array<HuntRecord,ArchiveQueueCapacity> archiveQueue;
size_t archiveHead=0,archiveCount=0;
unsigned archiveDropped=0;
std::atomic<bool> archiveEnabled{true};
bool loaded=false,pending=false;
bool initialHistoryTaken=false, pendingNeedsInitialHistory=false;
bool profilePending=false;
ProfileSettings pendingProfile{};
std::wstring storeFolder;
std::atomic<unsigned> completedWrites{0};
std::atomic<unsigned> completedProfiles{0};
std::atomic<unsigned> completedArchives{0};
std::atomic<bool> storageIdle{false};
DWORD WINAPI historyWorker(void*) {
  try {
    History history;
    const auto path=storeFolder+L"iceborne_damage_meter_history.dat";
    std::error_code error;
    const auto bytes=std::filesystem::file_size(path,error);
    if(!error && bytes<=128*1024) {
        std::ifstream in(std::filesystem::path(path),std::ios::binary);
        if(!history_codec::load(in,history)) log("History invalid or incompatible; starting empty.");
    }
    HuntArchive archive;
    const auto configurationPath=storeFolder+L"iceborne_damage_meter.ini";
    bool archiveReady=GetPrivateProfileIntW(L"Overlay",L"ArchiveHunts",1,configurationPath.c_str())!=0;
    if(archiveReady){
        try {
            archiveReady=archive.open(std::filesystem::path(storeFolder+L"iceborne_damage_meter_hunts.bin"));
            if(archiveReady){
                // Import only the existing ten recent records, never the full archive.
                unsigned imported=0;
                archiveReady=archive.importRecent(history,imported);
                completedArchives.fetch_add(imported);
                if(archive.recoveredBytes())log("Hunt archive recovered an interrupted tail (%llu bytes).",archive.recoveredBytes());
            }
            if(!archiveReady)log("Hunt archive disabled for this session: %s. Recent history remains active.",archive.error());
        } catch(...) {archiveReady=false;archive.close();log("Hunt archive initialization failed; recent history remains active.");}
    }
    archiveEnabled.store(archiveReady);
    { std::lock_guard<std::mutex> lock(storeMutex); loadedHistory=history; loaded=true;
      if(!pending&&!profilePending&&!archiveCount)storageIdle.store(true); }
    for(;;) {
        if(WaitForSingleObject(storeEvent,INFINITE)!=WAIT_OBJECT_0) return 0;
        for(;;){
        History toSave; HuntRecord last;ProfileSettings profile{};
        HuntRecord toArchive;bool writeArchive=false;unsigned dropped=0;
        bool writeHistory=false,writeProfile=false,mergeInitialHistory=false;
        { std::lock_guard<std::mutex> lock(storeMutex); if(!pending&&!profilePending&&!archiveCount&&!archiveDropped){storageIdle.store(true);break;}
          storageIdle.store(false);
          writeHistory=pending;writeProfile=profilePending;
          if(pending){toSave=pendingHistory;last=pendingLast;mergeInitialHistory=pendingNeedsInitialHistory;pending=false;}
          if(profilePending){profile=pendingProfile;profilePending=false;}
          if(archiveCount){toArchive=std::move(archiveQueue[archiveHead]);archiveHead=(archiveHead+1)%ArchiveQueueCapacity;--archiveCount;writeArchive=true;}
          dropped=archiveDropped;archiveDropped=0; }
        if(dropped)log("Hunt archive queue full: %u records could not be queued. Recent history remains active.",dropped);
        if(writeArchive&&archiveReady){
            try {
                const auto result=archive.append(toArchive);
                if(result==HuntArchive::Result::Appended)++completedArchives;
                if(result==HuntArchive::Result::Failed){
                    archiveReady=false;archiveEnabled.store(false);
                    log("Hunt archive disabled for this session: %s. Recent history remains active.",archive.error());
                }
            } catch(...) {archiveReady=false;archiveEnabled.store(false);archive.close();log("Hunt archive write failed; recent history remains active.");}
        }
        if(writeHistory) {
        // An early snapshot contains only hunts observed before the overlay
        // received its initial history. Preserve the original on-disk records.
        if (mergeInitialHistory) {
            auto combined = history;
            combined.addNewer(toSave);
            toSave = std::move(combined);
        }
        const auto temporary=path+L".tmp";
        bool ok=false;
        { std::ofstream out(std::filesystem::path(temporary),std::ios::binary|std::ios::trunc);
          ok=history_codec::save(out,toSave); out.flush(); ok=ok&&bool(out); }
        if(ok) ok=MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
        if(!ok) log("Could not save hunt history.");
        FILE* file=nullptr;
        if(!_wfopen_s(&file,(storeFolder+L"iceborne_damage_meter_last_hunt.txt").c_str(),L"wt, ccs=UTF-8") && file) {
            fwprintf(file,L"%ls\n%ls: %ls (#%d)\n%ls: %ls\n%ls: %llu\n%ls: %.1f s\n",
                tr(L"HUNT LOG"),tr(L"Quest"),questName(last.questId).c_str(),last.questId,
                tr(L"Objective"),questObjective(last.questId).c_str(),tr(L"Total damage"),last.total(),tr(L"Observed duration"),last.durationMs/1000.0);
            for(const auto& p:last.players) if(p.id) fwprintf(file,L"%ls\t%ls\t%u\t%.1f%%\t%.1f DPS\t%.1f s\n",
                p.name.c_str(),weaponStyle(p.weapon).name,p.damage,last.total()?100.0*p.damage/last.total():0,p.dps(),p.participationMs/1000.0);
            fwprintf(file,L"\n%ls\n",tr(L"DPS since the first observation; earlier damage is excluded."));
            fclose(file);
        }
        if(ok)++completedWrites;
        }
        if(writeProfile) {
            wchar_t number[32]{};bool ok=true;
            const auto ini=storeFolder+L"iceborne_damage_meter.ini";
            for(const auto& setting:profile)if(setting.key) {
                swprintf_s(number,L"%d",setting.value);
                if(!WritePrivateProfileStringW(L"Overlay",setting.key,number,ini.c_str()))ok=false;
            }
            if(ok)++completedProfiles;else log("Could not save overlay settings.");
        }
        }
    }
  } catch(...) {log("History storage exception; in-memory history remains available.");return 0;}
}
}
void startHistoryStore(const std::wstring& directory) {
    storeFolder=directory;
    storeEvent=CreateEventW(nullptr,FALSE,FALSE,nullptr);
    if(!storeEvent) { log("History storage event failed."); return; }
    const auto thread=CreateThread(nullptr,0,historyWorker,nullptr,0,nullptr);
    if(thread) CloseHandle(thread); else log("History storage worker failed.");
}
bool takeLoadedHistory(History& history) {
    std::lock_guard<std::mutex> lock(storeMutex);
    if(!loaded) return false;
    history=std::move(loadedHistory); loaded=false; initialHistoryTaken=true; return true;
}
void saveHistoryAsync(const History& history,const HuntRecord& last) {
    if(!storeEvent) return;
    { std::lock_guard<std::mutex> lock(storeMutex); storageIdle.store(false);pendingHistory=history; pendingLast=last; pending=true;
      pendingNeedsInitialHistory = !initialHistoryTaken;
      if(archiveEnabled.load()){
          if(archiveCount<ArchiveQueueCapacity){archiveQueue[(archiveHead+archiveCount)%ArchiveQueueCapacity]=last;++archiveCount;}
          else ++archiveDropped;
      } }
    SetEvent(storeEvent);
}
void saveProfileAsync(const ProfileSettings& values) {
    if(!storeEvent)return;
    {std::lock_guard<std::mutex> lock(storeMutex);storageIdle.store(false);pendingProfile=values;profilePending=true;}
    SetEvent(storeEvent);
}
#ifdef METER_DIAGNOSTIC
unsigned diagnosticHistoryWrites(){return completedWrites.load();}
unsigned diagnosticProfileWrites(){return completedProfiles.load();}
unsigned diagnosticArchiveWrites(){return completedArchives.load();}
bool diagnosticStorageIdle(){return storageIdle.load();}
#endif
}
