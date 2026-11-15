#include "runtime.hpp"
#include "history.hpp"
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
bool loaded=false,pending=false;
bool profilePending=false;
std::array<ProfileSetting,16> pendingProfile{};
std::wstring storeFolder;
std::atomic<unsigned> completedWrites{0};
std::atomic<unsigned> completedProfiles{0};
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
    { std::lock_guard<std::mutex> lock(storeMutex); loadedHistory=std::move(history); loaded=true; }
    for(;;) {
        if(WaitForSingleObject(storeEvent,INFINITE)!=WAIT_OBJECT_0) return 0;
        History toSave; HuntRecord last;std::array<ProfileSetting,16> profile{};
        bool writeHistory=false,writeProfile=false;
        { std::lock_guard<std::mutex> lock(storeMutex); if(!pending&&!profilePending) continue;
          writeHistory=pending;writeProfile=profilePending;
          if(pending){toSave=pendingHistory;last=pendingLast;pending=false;}
          if(profilePending){profile=pendingProfile;profilePending=false;} }
        if(writeHistory) {
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
    history=std::move(loadedHistory); loaded=false; return true;
}
void saveHistoryAsync(const History& history,const HuntRecord& last) {
    if(!storeEvent) return;
    { std::lock_guard<std::mutex> lock(storeMutex); pendingHistory=history; pendingLast=last; pending=true; }
    SetEvent(storeEvent);
}
void saveProfileAsync(const std::array<ProfileSetting,16>& values) {
    if(!storeEvent)return;
    {std::lock_guard<std::mutex> lock(storeMutex);pendingProfile=values;profilePending=true;}
    SetEvent(storeEvent);
}
#ifdef METER_DIAGNOSTIC
unsigned diagnosticHistoryWrites(){return completedWrites.load();}
unsigned diagnosticProfileWrites(){return completedProfiles.load();}
#endif
}
