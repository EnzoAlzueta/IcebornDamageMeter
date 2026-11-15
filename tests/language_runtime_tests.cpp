#include "../src/runtime.hpp"
#include "../src/language.hpp"
#include <filesystem>
#include <cstdio>
namespace meter {void log(const char* text,...){std::printf("Language runtime: %s\n",text);}}
int wmain(int argc,wchar_t** argv) {
    using namespace meter;
    if(argc!=2)return 1;
    const auto directory=std::filesystem::current_path()/(L"language-fixture-"+std::to_wstring(GetCurrentProcessId()));
    std::filesystem::create_directories(directory);
    const auto ini=(directory/L"iceborne_damage_meter.ini").wstring();
    if(!WritePrivateProfileStringW(L"Overlay",L"Language",L"auto",ini.c_str()))return 2;
    const auto steam=LoadLibraryW(argv[1]);if(!steam)return 3;
    initializeLanguage(directory.wstring()+L"\\");
    if(currentLanguage()==Language::Japanese)return 4;
    const auto ready=reinterpret_cast<void(*)()>(GetProcAddress(steam,"LanguageFixtureReady"));if(!ready)return 5;
    ready();Sleep(1100);updateGameLanguage();
    if(currentLanguage()!=Language::Japanese)return 6;
    if(!WritePrivateProfileStringW(L"Overlay",L"Language",L"fr",ini.c_str()))return 7;
    initializeLanguage(directory.wstring()+L"\\");updateGameLanguage();
    if(currentLanguage()!=Language::French)return 8;
    FreeLibrary(steam);
    std::puts("Automatic language: late Steam initialization and explicit override priority passed");
    return 0;
}
