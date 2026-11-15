#include "../src/process_com_ptr.hpp"
#include <cstdlib>
#include <cstdio>

namespace {
bool exiting = false;
unsigned releases = 0;
struct Object final : IUnknown {
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void**) override { return E_NOINTERFACE; }
    ULONG STDMETHODCALLTYPE AddRef() override { return 2; }
    ULONG STDMETHODCALLTYPE Release() override {
        if (exiting) {
            std::fputs("COM Release called during static process teardown\n", stderr);
            std::_Exit(91);
        }
        ++releases;
        return 1;
    }
} object;
meter::ProcessComPtr<IUnknown> global;
}
int main() {
    {
        Microsoft::WRL::ComPtr<IUnknown> local;
        local.Attach(&object);
    }
    global.Attach(&object);
    global.Reset();
    if (releases != 2) return 1;
    global.Attach(&object);
    // Registered last: executes before global's destructor after main returns.
    if (std::atexit([] { exiting = true; }) != 0) return 2;
    return 0;
}
