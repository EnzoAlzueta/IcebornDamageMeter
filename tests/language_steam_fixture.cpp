#include <atomic>
namespace {std::atomic<bool> ready{false};int context=0;}
extern "C" __declspec(dllexport) int SteamAPI_GetHSteamUser(){return ready?1:0;}
extern "C" __declspec(dllexport) void* SteamAPI_SteamApps_v008(){return ready?&context:nullptr;}
extern "C" __declspec(dllexport) const char* SteamAPI_ISteamApps_GetCurrentGameLanguage(void* apps){return apps==&context?"japanese":"";}
extern "C" __declspec(dllexport) void LanguageFixtureReady(){ready=true;}
