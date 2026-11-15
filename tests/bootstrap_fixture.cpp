#include <windows.h>
namespace { LONG attaches = 0; }
extern "C" __declspec(dllexport) LONG IdmFixtureAttachCount() { return attaches; }
BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) InterlockedIncrement(&attaches);
    return TRUE;
}
