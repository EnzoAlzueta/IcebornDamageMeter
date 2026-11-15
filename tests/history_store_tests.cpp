#include "../src/runtime.hpp"
#include <filesystem>
#include <fstream>
#include <cstdio>
namespace meter {
void log(const char* text,...){std::printf("Storage: %s\n",text);}
unsigned diagnosticHistoryWrites();
unsigned diagnosticProfileWrites();
}
int main(){
    using namespace meter;
    const auto directory=std::filesystem::current_path()/(L"storage-test-"+std::to_wstring(GetCurrentProcessId()));
    std::filesystem::create_directories(directory);
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
    std::puts("Background storage: history load, atomic save, summary and settings passed.");return 0;
}
