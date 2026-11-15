#pragma once
#include <dxgi1_4.h>
#include <array>
#include <algorithm>
#include <stdexcept>

// An independent overlay snapshots the previous Present and installs a private
// table. This models the observed Steam/IDM cycle without loading Steam or a game.
namespace overlay_fixture {
using Present=HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*,UINT,UINT);
inline Present previous=nullptr;
inline unsigned calls=0;
inline thread_local unsigned depth=0;
__declspec(noinline) inline HRESULT STDMETHODCALLTYPE present(IDXGISwapChain* chain,UINT interval,UINT flags) {
    struct Guard{Guard(){if(++depth>8)throw std::runtime_error("External overlay/IDM recursive Present chain");}~Guard(){--depth;}} guard;
    ++calls;
    return previous(chain,interval,flags);
}
struct Overlay {
    std::array<void*,40> table{};
    void*** object=nullptr;
    void** runtime=nullptr;
    ~Overlay(){restore();}
    void install(void* chain) {
        object=static_cast<void***>(chain);runtime=*object;
        std::copy_n(runtime,table.size(),table.begin());
        previous=reinterpret_cast<Present>(table[8]);table[8]=reinterpret_cast<void*>(&present);
        *object=table.data();
    }
    void restore(){if(object){*object=runtime;object=nullptr;}}
};
}
