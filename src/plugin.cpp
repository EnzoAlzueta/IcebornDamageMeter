#include "runtime.hpp"
#include "version.hpp"
#include "language.hpp"
#include <cstdio>
#include <cstdarg>
namespace meter {
HMODULE module = nullptr;
std::wstring folder;
std::atomic<bool> supported{false}, stopping{false};
namespace { std::mutex logMutex; }
void log(const char* format, ...) {
    char text[2048]{};
    va_list args;
    va_start(args, format);
    vsnprintf_s(text, sizeof(text), _TRUNCATE, format, args);
    va_end(args);
    std::lock_guard<std::mutex> lock(logMutex);
    FILE* file = nullptr;
    if (_wfopen_s(&file, (folder + L"iceborne_damage_meter.log").c_str(), L"ab") || !file) return;
    fprintf(file, "[%llu] %s\n", GetTickCount64(), text);
    fclose(file);
}
DWORD WINAPI start(void*) {
    try {
        wchar_t path[32768]{};
        if (!GetModuleFileNameW(module, path, 32768)) return 0;
        folder = path;
        folder.resize(folder.find_last_of(L"\\/") + 1);
        log("Iceborne Damage Meter %s starting (internal, automatic DX11/DX12)",IDM_VERSION);
        initializeLanguage(folder);
        startHistoryStore(folder);
        // Pin the module while its callbacks are registered.
        HMODULE pinned = nullptr;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
            reinterpret_cast<LPCWSTR>(&start), &pinned);
        if (!installHooks()) { removeHooks(); log("Could not hook DXGI; plugin inactive."); return 0; }
        log("DXGI discovery ready; waiting for the game's swapchain/Present.");
        initializeReader();
        log("Initialization complete. Automatic mission visibility; F8 manual; Ctrl+F9 drag; Ctrl+F8 preview.");
    } catch (...) { log("Initialization exception; plugin inactive."); }
    return 0;
}
}
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        meter::module = instance;
        // No DirectX, file I/O or waiting under the Windows loader lock.
        HANDLE thread = CreateThread(nullptr, 0, meter::start, nullptr, 0, nullptr);
        if (thread) CloseHandle(thread);
    } else if (reason == DLL_PROCESS_DETACH) {
        meter::stopping = true;
        // Process teardown owns COM/heap cleanup; never wait in DllMain.
    }
    return TRUE;
}
