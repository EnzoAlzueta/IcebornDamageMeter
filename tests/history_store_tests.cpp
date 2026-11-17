#include "../src/runtime.hpp"
#include "../src/hunt_archive.hpp"
#include <filesystem>
#include <fstream>
#include <cstdio>
#include <cstring>
namespace meter {
void log(const char* text,...){std::printf("Storage: %s\n",text);}
unsigned diagnosticHistoryWrites();
unsigned diagnosticProfileWrites();
unsigned diagnosticArchiveWrites();
bool diagnosticStorageIdle();
}
int main(int argc,char** argv){
    using namespace meter;
    const bool disabled=argc>1&&std::string(argv[1])=="disabled",failure=argc>1&&std::string(argv[1])=="failure";
    const auto directory=std::filesystem::current_path()/(L"storage-test-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));
    std::filesystem::create_directories(directory);
    const auto archive=directory/L"iceborne_damage_meter_hunts.bin";
    if(disabled){std::ofstream out(directory/L"iceborne_damage_meter.ini");out<<"[Overlay]\nArchiveHunts=0\n";}
    if(failure)std::filesystem::create_directory(archive);
    HuntRecord r;r.timestamp=1000;r.questId=42;r.outcome=3;r.partySize=1;r.durationMs=10000;
    r.players[0]={1,L"Jugador español",100,true,true,0};r.players[0].participationMs=10000;
    History history;history.add(r);
    const auto path=directory/L"iceborne_damage_meter_history.dat";
    {std::ofstream out(path,std::ios::binary);if(!history_codec::save(out,history))return 1;}
    startHistoryStore(directory.wstring()+L"\\");
    History loaded;bool ready=false;
    for(int i=0;i<200&&!ready;++i){ready=takeLoadedHistory(loaded);if(!ready)Sleep(5);}
    if(!ready||loaded.size()!=1||loaded.get(0)->players[0].name!=r.players[0].name)return 2;
    r.timestamp=2000;history.add(r);saveHistoryAsync(history,r);
    std::array<ProfileSetting,16> config{};
    config[0]={L"ScalePercent",125};config[1]={L"PositionX",2000};config[2]={L"ShowDps",0};
    saveProfileAsync(config);
    for(int i=0;i<400&&!diagnosticHistoryWrites();++i)Sleep(5);
    if(!diagnosticHistoryWrites())return 3;
    {std::ifstream in(path,std::ios::binary);if(!history_codec::load(in,loaded))return 4;}
    if(loaded.size()!=2||loaded.get(0)->timestamp!=2000)return 5;
    if(!std::filesystem::exists(directory/L"iceborne_damage_meter_last_hunt.txt"))return 6;
    for(int i=0;i<400&&!diagnosticProfileWrites();++i)Sleep(5);
    const auto ini=(directory/L"iceborne_damage_meter.ini").wstring();
    if(!diagnosticProfileWrites()||GetPrivateProfileIntW(L"Overlay",L"ScalePercent",0,ini.c_str())!=125||
        GetPrivateProfileIntW(L"Overlay",L"PositionX",0,ini.c_str())!=2000||
        GetPrivateProfileIntW(L"Overlay",L"ShowDps",1,ini.c_str())!=0)return 7;
    // Several queued hunts must survive even if the ten-entry snapshot is coalesced.
    for(unsigned i=0;i<20;++i){r.timestamp=3000+i;history.add(r);saveHistoryAsync(history,r);}
    for(unsigned i=0;i<2000&&!diagnosticStorageIdle();++i)Sleep(5);
    if(!diagnosticStorageIdle())return 8;
    {std::ifstream in(path,std::ios::binary);if(!history_codec::load(in,loaded)||loaded.size()!=10||loaded.get(0)->timestamp!=3019)return 9;}
    if(disabled){if(std::filesystem::exists(archive)||diagnosticArchiveWrites()!=0)return 10;}
    else if(failure){if(!std::filesystem::is_directory(archive)||diagnosticArchiveWrites()!=0)return 11;}
    else {
        if(diagnosticArchiveWrites()!=22)return 12;
        std::ifstream in(archive,std::ios::binary);
        std::string data{std::istreambuf_iterator<char>(in),{}};size_t position=archive_codec::HeaderBytes;unsigned count=0;
        while(position<data.size()){
            if(position+8>data.size()||data.compare(position,4,"HNT1"))return 13;
            std::uint32_t length=0;std::memcpy(&length,data.data()+position+4,4);
            if(length>archive_codec::MaxPayloadBytes||position+length+archive_codec::FrameOverhead>data.size())return 14;
            HuntRecord saved;if(!archive_codec::decode(std::string_view(data).substr(position+8,length),saved))return 15;
            const auto expected=count==0?1000:count==1?2000:3000+count-2;
            if(saved.timestamp!=expected)return 16;
            position+=length+archive_codec::FrameOverhead;++count;
        }
        if(count!=22)return 17;
    }
    std::puts("Background storage: history load, atomic save, summary and settings passed.");return 0;
}
