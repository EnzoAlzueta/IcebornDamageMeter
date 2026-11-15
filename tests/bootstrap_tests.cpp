#include <windows.h>
#include <Xinput.h>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <fstream>

namespace fs = std::filesystem;
int main(int argc, char** argv) {
    const bool wrongHost = argc > 1 && std::string(argv[1]) == "--wrong-host";
    const bool missing = argc > 1 && std::string(argv[1]) == "--missing";
    wchar_t executable[32768]{};
    GetModuleFileNameW(nullptr, executable, 32768);
    const fs::path root = fs::path(executable).parent_path();
    const fs::path pluginPath = root / L"nativePC/plugins/iceborne_damage_meter.dll";
    // XInput is imported by this executable: the proxy starts before main().
    HMODULE proxy = GetModuleHandleW(L"xinput9_1_0.dll");
    wchar_t proxyPath[32768]{};
    if (!proxy || !GetModuleFileNameW(proxy, proxyPath, 32768) ||
        !fs::equivalent(fs::path(proxyPath), root / L"xinput9_1_0.dll")) return 1;
    wchar_t system[MAX_PATH]{};
    if (!GetSystemDirectoryW(system, MAX_PATH)) return 2;
    const auto realPath = fs::path(system) / L"xinput9_1_0.dll";
    HMODULE real = LoadLibraryExW(realPath.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!real) return 3;
    using State = DWORD (WINAPI*)(DWORD, XINPUT_STATE*);
    using Set = DWORD (WINAPI*)(DWORD, XINPUT_VIBRATION*);
    using Caps = DWORD (WINAPI*)(DWORD, DWORD, XINPUT_CAPABILITIES*);
    using Audio = DWORD (WINAPI*)(DWORD, GUID*, GUID*);
    const auto state = reinterpret_cast<State>(GetProcAddress(real,"XInputGetState"));
    const auto set = reinterpret_cast<Set>(GetProcAddress(real,"XInputSetState"));
    const auto caps = reinterpret_cast<Caps>(GetProcAddress(real,"XInputGetCapabilities"));
    const auto audio = reinterpret_cast<Audio>(GetProcAddress(real,"XInputGetDSoundAudioDeviceGuids"));
    const auto proxyAudio = reinterpret_cast<Audio>(GetProcAddress(proxy,"XInputGetDSoundAudioDeviceGuids"));
    if (!state || !set || !caps || !audio || !proxyAudio) return 4;
    const char* names[] = {"XInputGetCapabilities","XInputGetDSoundAudioDeviceGuids","XInputGetState","XInputSetState"};
    for (WORD i = 0; i < 4; ++i) {
        if (GetProcAddress(proxy,names[i]) != GetProcAddress(proxy,MAKEINTRESOURCEA(i+2))) return 5;
    }
    std::atomic<bool> ok{true};
    std::vector<std::thread> threads;
    // Exercise concurrent first calls, then steady-state cached forwarding.
    // Invalid user indices cannot vibrate or otherwise affect a real controller.
    for (int i = 0; i < 8; ++i) threads.emplace_back([&] {
        for (int j = 0; j < 100; ++j) {
            XINPUT_STATE a{}, b{};
            XINPUT_CAPABILITIES c{}, d{};
            XINPUT_VIBRATION v{};
            GUID render{}, capture{};
            if (XInputGetState(99,&a) != state(99,&b) ||
                XInputSetState(99,&v) != set(99,&v) ||
                XInputGetCapabilities(99,0,&c) != caps(99,0,&d) ||
                proxyAudio(99,&render,&capture) != audio(99,&render,&capture)) ok = false;
        }
    });
    for (auto& thread : threads) thread.join();
    if (!ok) return 6;
    const auto logPath = root / L"iceborne_damage_meter_bootstrap.log";
    if (wrongHost) {
        Sleep(150);
        if (GetModuleHandleW(pluginPath.c_str()) || fs::exists(logPath)) return 7;
    } else {
        const ULONGLONG deadline = GetTickCount64() + 5000;
        std::string content;
        while (GetTickCount64() < deadline) {
            std::ifstream log(logPath);
            content.assign(std::istreambuf_iterator<char>(log), {});
            if (content.find('\n') != std::string::npos) break;
            Sleep(10);
        }
        if (missing) {
            if (GetModuleHandleW(pluginPath.c_str()) || content.find("could not be loaded") == std::string::npos) return 8;
        } else {
            // Simulate an existing loader loading the same plugin as well.
            HMODULE first = LoadLibraryExW(pluginPath.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
            HMODULE second = LoadLibraryW(pluginPath.c_str());
            if (!first || first != second || first != GetModuleHandleW(pluginPath.c_str())) return 9;
            const auto count = reinterpret_cast<LONG(*)()>(GetProcAddress(first,"IdmFixtureAttachCount"));
            if (!count || count() != 1 || content.find("plugin loaded") == std::string::npos) return 10;
        }
    }
    std::cout << "Bootstrap: import loading, system forwarding, ordinals, concurrency and plugin isolation passed.\n";
    return 0;
}
