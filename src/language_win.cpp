#include "language.hpp"
#include "runtime.hpp"
#include <filesystem>
#include <fstream>
#include <cstring>
namespace meter {
namespace {
bool automatic=true,resolved=false;
ULONGLONG nextProbe=0,probeUntil=0;
std::string utf8(const wchar_t* value) {
    char bytes[128]{};
    const int count=WideCharToMultiByte(CP_UTF8,0,value,-1,bytes,sizeof(bytes),nullptr,nullptr);
    return count>0?std::string(bytes):std::string{};
}
std::string gameLanguageFromSteam() {
    // Read the game's already-initialized Steam context; never initialize Steam
    // or load/replace its DLL. Use documented flat exports, not vtable offsets.
    const auto steam=GetModuleHandleW(L"steam_api64.dll");if(!steam)return {};
    using User=int(__cdecl*)();using Apps=void*(__cdecl*)();using GameLanguage=const char*(__cdecl*)(void*);
    const auto user=reinterpret_cast<User>(GetProcAddress(steam,"SteamAPI_GetHSteamUser"));
    const auto apps=reinterpret_cast<Apps>(GetProcAddress(steam,"SteamAPI_SteamApps_v008"));
    const auto get=reinterpret_cast<GameLanguage>(GetProcAddress(steam,"SteamAPI_ISteamApps_GetCurrentGameLanguage"));
    if(!user||!apps||!get||!user())return {};
    const auto instance=apps();if(!instance)return {};
    const auto value=get(instance);if(!value)return {};
    const auto length=strnlen_s(value,64);return length&&length<64?std::string(value,length):std::string{};
}
std::string gameLanguageFromManifest() {
    wchar_t executable[32768]{};
    if(!GetModuleFileNameW(nullptr,executable,32768))return {};
    const auto path=std::filesystem::path(executable).parent_path().parent_path().parent_path()/L"appmanifest_582010.acf";
    std::ifstream stream(path,std::ios::binary);if(!stream)return {};
    std::string text(256*1024,'\0');stream.read(text.data(),static_cast<std::streamsize>(text.size()));
    text.resize(static_cast<size_t>(stream.gcount()));return manifestLanguage(text);
}
}
void initializeLanguage(const std::wstring& directory) {
    wchar_t overrideName[64]{};
    GetPrivateProfileStringW(L"Overlay",L"Language",L"auto",overrideName,64,(directory+L"iceborne_damage_meter.ini").c_str());
    const auto overrideValue=utf8(overrideName);
    automatic=true;resolved=false;nextProbe=0;probeUntil=GetTickCount64()+60000;
    if(!overrideValue.empty()&&_stricmp(overrideValue.c_str(),"auto")) {
        if(const auto chosen=parseLanguage(overrideValue)) {
            setLanguage(*chosen);automatic=false;
            log("UI language: %s (explicit INI override)",languageCode());return;
        }
        log("Unrecognized UI language override; using game language automatically.");
    }
    wchar_t locale[LOCALE_NAME_MAX_LENGTH]{};GetUserDefaultLocaleName(locale,LOCALE_NAME_MAX_LENGTH);
    const auto manifest=gameLanguageFromManifest();
    setLanguage(selectAutomaticLanguage({},manifest,utf8(locale)));
    log("UI language: %s (%s; game Steam API pending)",languageCode(),parseLanguage(manifest)?"game manifest":"OS/English fallback");
    updateGameLanguage();
}
void updateGameLanguage() {
    if(!automatic||resolved)return;
    const auto now=GetTickCount64();if(now<nextProbe)return;
    nextProbe=now+1000;
    if(probeUntil&&now>probeUntil){resolved=true;log("Game language API not ready; retaining %s.",languageCode());return;}
    if(const auto chosen=parseLanguage(gameLanguageFromSteam())) {
        setLanguage(*chosen);resolved=true;
        log("UI language: %s (Steam current GAME language)",languageCode());
    }
}
}
