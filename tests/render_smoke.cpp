#include "../src/runtime.hpp"
#include "../src/language.hpp"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <cstdio>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <string>
#include <cmath>
#include "external_overlay.hpp"
using Microsoft::WRL::ComPtr;
namespace meter {
void diagnosticPreview(bool allWeapons, bool zeroDamage);
bool diagnosticReady();
void diagnosticRelativeMotion(LONG dx, LONG dy);
unsigned diagnosticCacheBuilds();
void diagnosticPanelPage(int page);
void diagnosticPanelSize(int,int);
void diagnosticHidden(bool);
void diagnosticRecapture(void*,IUnknown*);
}
static void require(HRESULT result, const char* operation) {
    if (FAILED(result)) {
        std::printf("%s: 0x%08lx\n", operation, static_cast<unsigned long>(result));
        throw std::runtime_error(operation);
    }
}
static bool mouseRegistration(RAWINPUTDEVICE& mouse) {
    UINT count = 0;
    if (GetRegisteredRawInputDevices(nullptr, &count, sizeof(mouse)) == static_cast<UINT>(-1)) throw std::runtime_error("Raw registration count");
    std::vector<RAWINPUTDEVICE> devices(count);
    if (count && GetRegisteredRawInputDevices(devices.data(), &count, sizeof(mouse)) == static_cast<UINT>(-1)) throw std::runtime_error("Raw registration list");
    for (const auto& device : devices) if (device.usUsagePage == 1 && device.usUsage == 2) { mouse = device; return true; }
    return false;
}
static void pump(HWND window) {
    MSG message{};
    while (PeekMessageW(&message, window, 0, 0, PM_REMOVE)) { TranslateMessage(&message); DispatchMessageW(&message); }
}
static void exerciseEditing(HWND window) {
    RAWINPUTDEVICE before{}, during{}, after{};
    const bool originallyRegistered = mouseRegistration(before);
    meter::setEditing(true); pump(window);
    if (!mouseRegistration(during) || during.hwndTarget != window || !(during.dwFlags & RIDEV_INPUTSINK))
        throw std::runtime_error("Edit Raw Input not registered");
    POINT point{}; bool left = false;
    if (!meter::getEditPointer(point, left)) throw std::runtime_error("Edit pointer inactive");
    const LONG dx = point.x >= 80 ? -80 : 80, dy = point.y >= 60 ? -60 : 60;
    meter::diagnosticRelativeMotion(dx, dy);
    POINT moved{};
    if (!meter::getEditPointer(moved, left) || moved.x != point.x+dx || moved.y != point.y+dy)
        throw std::runtime_error("Relative pointer did not move independently of OS cursor");
    meter::setEditing(false); pump(window);
    const auto verifyRestored = [&] {
        const bool registered = mouseRegistration(after);
        if (registered != originallyRegistered || (registered &&
            (before.hwndTarget != after.hwndTarget || before.dwFlags != after.dwFlags)))
            throw std::runtime_error("Game Raw Input registration not restored");
    };
    verifyRestored();
    meter::setEditing(true); pump(window);
    SendMessageW(window, WM_KILLFOCUS, 0, 0); pump(window);
    if (meter::isEditing()) throw std::runtime_error("Focus loss did not release edit mode");
    verifyRestored();
    std::puts("Edit pointer: registration, restoration and focus-loss cleanup passed.");
}
static void testEditing(HWND window) {
    RAWINPUTDEVICE original{};
    const bool hadOriginal = mouseRegistration(original);
    exerciseEditing(window);
    // Also test the branch used when a game already has its own mouse target.
    const RAWINPUTDEVICE existing{1, 2, RIDEV_INPUTSINK | RIDEV_NOLEGACY, window};
    if (!RegisterRawInputDevices(&existing, 1, sizeof(existing))) throw std::runtime_error("Seed Raw Input registration");
    exerciseEditing(window);
    const RAWINPUTDEVICE remove{1, 2, RIDEV_REMOVE, nullptr};
    const auto restore = hadOriginal ? original : remove;
    if (!RegisterRawInputDevices(&restore, 1, sizeof(restore))) throw std::runtime_error("Restore harness mouse registration");
}
static void savePreview(ID3D12Device* device, ID3D12CommandQueue* queue, IDXGISwapChain1* chain, UINT index, const char* fileName,int expected=-1) {
    ComPtr<ID3D12Resource> source, readback;
    require(chain->GetBuffer(index, IID_PPV_ARGS(&source)), "Preview GetBuffer");
    const auto sourceDesc = source->GetDesc();
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
    UINT64 bytes = 0;
    device->GetCopyableFootprints(&sourceDesc, 0, 1, 0, &footprint, nullptr, nullptr, &bytes);
    D3D12_HEAP_PROPERTIES heap{}; heap.Type = D3D12_HEAP_TYPE_READBACK;
    D3D12_RESOURCE_DESC desc{}; desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = bytes; desc.Height = 1; desc.DepthOrArraySize = 1; desc.MipLevels = 1;
    desc.SampleDesc.Count = 1; desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    require(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr, IID_PPV_ARGS(&readback)), "Preview readback");
    ComPtr<ID3D12CommandAllocator> allocator;
    ComPtr<ID3D12GraphicsCommandList> list;
    require(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)), "Preview allocator");
    require(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&list)), "Preview command list");
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = source.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
    list->ResourceBarrier(1, &barrier);
    D3D12_TEXTURE_COPY_LOCATION from{}, to{};
    from.pResource = source.Get(); from.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    to.pResource = readback.Get(); to.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT; to.PlacedFootprint = footprint;
    list->CopyTextureRegion(&to, 0, 0, 0, &from, nullptr);
    std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter);
    list->ResourceBarrier(1, &barrier);
    require(list->Close(), "Preview close");
    ID3D12CommandList* lists[] = {list.Get()}; queue->ExecuteCommandLists(1, lists);
    ComPtr<ID3D12Fence> fence;
    require(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "Preview fence");
    HANDLE event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!event) throw std::runtime_error("Preview event");
    require(queue->Signal(fence.Get(), 1), "Preview signal");
    require(fence->SetEventOnCompletion(1, event), "Preview fence completion");
    const auto wait = WaitForSingleObject(event, 10000); CloseHandle(event);
    if (wait != WAIT_OBJECT_0) throw std::runtime_error("Preview timeout");
    void* pixels = nullptr; D3D12_RANGE range{0, static_cast<SIZE_T>(bytes)};
    require(readback->Map(0, &range, &pixels), "Preview map");
    const auto width = static_cast<UINT>(sourceDesc.Width), height = sourceDesc.Height;
    std::vector<BYTE> bitmap(static_cast<size_t>(width)*height*4);
    unsigned changed=0;
    for (UINT y = 0; y < height; ++y) for (UINT x = 0; x < width; ++x) {
        const auto input = static_cast<BYTE*>(pixels) + y*footprint.Footprint.RowPitch + x*4;
        auto output = bitmap.data() + (static_cast<size_t>(y)*width+x)*4;
        output[0] = input[2]; output[1] = input[1]; output[2] = input[0]; output[3] = 255;
        if(input[0]>8||input[1]>8||input[2]>8)++changed;
    }
    readback->Unmap(0, nullptr);
    if(expected==0&&changed)throw std::runtime_error("Hidden DX12 meter still rendered pixels");
    if(expected==1&&changed<300)throw std::runtime_error("DX12 meter did not recover after visibility toggle");
    if(!fileName)return;
    BITMAPFILEHEADER fileHeader{}; fileHeader.bfType = 0x4d42;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
    fileHeader.bfSize = fileHeader.bfOffBits + static_cast<DWORD>(bitmap.size());
    BITMAPINFOHEADER info{}; info.biSize = sizeof(info); info.biWidth = static_cast<LONG>(width);
    info.biHeight = -static_cast<LONG>(height); info.biPlanes = 1; info.biBitCount = 32;
    FILE* file = nullptr; fopen_s(&file, fileName, "wb");
    if (!file) throw std::runtime_error("Preview file");
    fwrite(&fileHeader, sizeof(fileHeader), 1, file); fwrite(&info, sizeof(info), 1, file);
    fwrite(bitmap.data(), 1, bitmap.size(), file); fclose(file);
}
int main(int argc, char** argv) {
    const bool copied = argc > 1 && std::strstr(argv[1], "copied") != nullptr;
    const bool externalBefore=argc>1&&std::strstr(argv[1],"external-before")!=nullptr;
    const bool externalAfter=argc>1&&std::strstr(argv[1],"external-after")!=nullptr;
    const bool late = externalBefore||(argc > 1 && std::strstr(argv[1], "late") != nullptr);
    const bool gallery = argc > 1 && std::strcmp(argv[1], "--gallery") == 0;
    const bool zeroDamage = argc > 1 && std::strcmp(argv[1], "--zero") == 0;
    const bool settings = argc > 1 && std::strstr(argv[1], "settings") != nullptr;
    const bool history = argc > 1 && std::strstr(argv[1], "history") != nullptr;
    const bool narrow = argc > 1 && std::strstr(argv[1], "narrow") != nullptr;
    const bool tinyScale = argc > 1 && std::strcmp(argv[1], "--small") == 0;
    const bool localeJapanese=argc>1&&std::strcmp(argv[1],"--locale-ja")==0;
    const bool localeArabic=argc>1&&std::strcmp(argv[1],"--locale-ar")==0;
    const bool localeSwitch=argc>1&&std::strcmp(argv[1],"--locale-switch")==0;
    const bool hiddenTest=argc>1&&std::strcmp(argv[1],"--hidden")==0;
    const bool record = argc > 1 && std::strcmp(argv[1], "--record") == 0;
    const bool menu = argc > 1 && std::strcmp(argv[1], "--menu") == 0;
    HWND window = nullptr;
    HANDLE event = nullptr;
    try {
        wchar_t directory[32768]{};
        GetCurrentDirectoryW(32768, directory);
        meter::folder = std::wstring(directory) + L"\\";
        WNDCLASSW wc{};
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"IceborneMeterSmoke";
        if (!RegisterClassW(&wc)) throw std::runtime_error("RegisterClass");
        window = CreateWindowW(wc.lpszClassName, L"Meter diagnostic (hidden)", WS_OVERLAPPEDWINDOW,
            0, 0, 800, 600, nullptr, nullptr, wc.hInstance, nullptr);
        if (!window) throw std::runtime_error("CreateWindow");
        if (!late && !meter::installHooks()) throw std::runtime_error("installHooks");
        ComPtr<IDXGIFactory4> factory;
        require(CreateDXGIFactory1(IID_PPV_ARGS(&factory)), "CreateDXGIFactory1");
        ComPtr<IDXGIAdapter> warp;
        require(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp)), "EnumWarpAdapter");
        ComPtr<ID3D12Device> device;
        require(D3D12CreateDevice(late ? nullptr : warp.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)), "D3D12CreateDevice");
        D3D12_COMMAND_QUEUE_DESC qdesc{};
        ComPtr<ID3D12CommandQueue> queue;
        require(device->CreateCommandQueue(&qdesc, IID_PPV_ARGS(&queue)), "CreateCommandQueue");
        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.Width = 800; desc.Height = 600; desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1; desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = 2; desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        ComPtr<IDXGISwapChain1> chain;
        if (late) {
            DXGI_SWAP_CHAIN_DESC legacy{};
            legacy.BufferDesc.Width = desc.Width; legacy.BufferDesc.Height = desc.Height;
            legacy.BufferDesc.Format = desc.Format; legacy.SampleDesc = desc.SampleDesc;
            legacy.BufferUsage = desc.BufferUsage; legacy.BufferCount = desc.BufferCount;
            legacy.OutputWindow = window; legacy.Windowed = TRUE; legacy.SwapEffect = desc.SwapEffect;
            ComPtr<IDXGISwapChain> older;
            require(factory->CreateSwapChain(queue.Get(), &legacy, &older), "CreateSwapChain legacy");
            require(older.As(&chain), "Legacy IDXGISwapChain1");
        } else {
            require(factory->CreateSwapChainForHwnd(queue.Get(), window, &desc, nullptr, nullptr, &chain), "CreateSwapChainForHwnd");
        }
        // Regression: plugin starts after both the device and swapchain exist.
        overlay_fixture::Overlay external;
        if(externalBefore)external.install(chain.Get());
        if (late && !meter::installHooks()) throw std::runtime_error("installHooks late");
        if(externalAfter){external.install(chain.Get());meter::diagnosticRecapture(chain.Get(),queue.Get());}
        // Simulate an overlay copying a table that already contains our detours.
        // The factory capture must not register our callback as its own original.
        static std::array<void*,40> proxyTable{};
        void** runtimeTable=nullptr;
        if(copied) {
            auto slot=reinterpret_cast<void***>(chain.Get());runtimeTable=*slot;
            std::copy_n(runtimeTable,proxyTable.size(),proxyTable.begin());
            *slot=proxyTable.data();
            meter::diagnosticRecapture(chain.Get(),queue.Get());
        }
        if(narrow||tinyScale)meter::diagnosticPanelSize(260,tinyScale?40:100);
        if(localeJapanese||localeArabic)meter::setLanguage(localeJapanese?meter::Language::Japanese:meter::Language::Arabic);
        meter::diagnosticPreview(gallery, zeroDamage);
        if(localeJapanese||localeArabic)meter::diagnosticPanelPage(1);
        if(settings||history||record||menu)meter::diagnosticPanelPage(settings?1:history?2:menu?4:3);
        ComPtr<ID3D12CommandAllocator> allocator;
        ComPtr<ID3D12GraphicsCommandList> commandList;
        require(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)), "CreateCommandAllocator");
        require(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr,
            IID_PPV_ARGS(&commandList)), "CreateCommandList");
        require(commandList->Close(), "Close");
        ComPtr<ID3D12Fence> fence;
        require(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "CreateFence");
        event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (!event) throw std::runtime_error("CreateEvent");
        ComPtr<ID3D12DescriptorHeap> visibilityHeap;
        if(hiddenTest){D3D12_DESCRIPTOR_HEAP_DESC heap{};heap.Type=D3D12_DESCRIPTOR_HEAP_TYPE_RTV;heap.NumDescriptors=1;
            require(device->CreateDescriptorHeap(&heap,IID_PPV_ARGS(&visibilityHeap)),"Visibility RTV heap");meter::diagnosticHidden(true);}
        for (UINT64 frame = 1; frame <= 12; ++frame) {
            const bool hiddenFrame=hiddenTest&&(frame<=2||(frame>=5&&frame<10));
            if(hiddenTest&&(frame==3||frame==5||frame==10))meter::diagnosticHidden(frame==5);
            if(localeSwitch&&frame==6)meter::setLanguage(meter::Language::Spanish);
            ComPtr<IDXGISwapChain3> indexed; require(chain.As(&indexed), "Indexed swapchain");
            const UINT renderedIndex = indexed->GetCurrentBackBufferIndex();
            if(hiddenTest){
                require(allocator->Reset(),"Visibility allocator reset");require(commandList->Reset(allocator.Get(),nullptr),"Visibility list reset");
                ComPtr<ID3D12Resource> back;require(chain->GetBuffer(renderedIndex,IID_PPV_ARGS(&back)),"Visibility backbuffer");
                const auto rtv=visibilityHeap->GetCPUDescriptorHandleForHeapStart();device->CreateRenderTargetView(back.Get(),nullptr,rtv);
                D3D12_RESOURCE_BARRIER barrier{};barrier.Type=D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;barrier.Transition.pResource=back.Get();
                barrier.Transition.Subresource=D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;barrier.Transition.StateBefore=D3D12_RESOURCE_STATE_PRESENT;barrier.Transition.StateAfter=D3D12_RESOURCE_STATE_RENDER_TARGET;
                commandList->ResourceBarrier(1,&barrier);const float clear[4]={0,0,0,1};commandList->ClearRenderTargetView(rtv,clear,0,nullptr);
                std::swap(barrier.Transition.StateBefore,barrier.Transition.StateAfter);commandList->ResourceBarrier(1,&barrier);require(commandList->Close(),"Visibility list close");
            }
            ID3D12CommandList* lists[] = {commandList.Get()};
            queue->ExecuteCommandLists(1, lists);
            require(chain->Present(0, 0), "Present");
            require(queue->Signal(fence.Get(), frame), "Signal");
            require(fence->SetEventOnCompletion(frame, event), "SetEventOnCompletion");
            if (WaitForSingleObject(event, 10000) != WAIT_OBJECT_0) throw std::runtime_error("GPU timeout");
            if (!hiddenFrame&&!meter::diagnosticReady()) throw std::runtime_error("Overlay did not initialize");
            if(hiddenTest&&(frame==1||frame==2||frame==3||frame==5||frame==9||frame==10))savePreview(device.Get(),queue.Get(),chain.Get(),renderedIndex,nullptr,hiddenFrame?0:1);
            if (frame == 1 && !late&&!hiddenTest) savePreview(device.Get(), queue.Get(), chain.Get(), renderedIndex,
                narrow||tinyScale||localeJapanese||localeArabic||localeSwitch?(std::string("preview-dx12-")+argv[1]+".bmp").c_str():settings?"preview-settings.bmp":history?"preview-history.bmp":record?"preview-record.bmp":menu?"preview-menu.bmp":
                zeroDamage ? "preview-zero.bmp" : gallery?"preview.bmp":"preview-compact.bmp");
            if (frame == 4) require(chain->ResizeBuffers(2, 1024, 768, DXGI_FORMAT_R8G8B8A8_UNORM, 0), "ResizeBuffers");
            if (frame == 8) require(chain->ResizeBuffers(2, 800, 600, DXGI_FORMAT_R8G8B8A8_UNORM, 0), "ResizeBuffers back");
        }
        if(hiddenTest?meter::diagnosticCacheBuilds()<2:meter::diagnosticCacheBuilds()!=(localeSwitch?2U:1U))throw std::runtime_error("Unexpected UI cache rebuild count");
        std::printf("UI cache: %u build(s) across 12 frames and two resizes.\n",meter::diagnosticCacheBuilds());
        testEditing(window);
        meter::shutdownOverlay();
        meter::removeHooks();
        external.restore();
        if((externalBefore||externalAfter)&&!overlay_fixture::calls)throw std::runtime_error("External overlay was bypassed");
        if(runtimeTable)*reinterpret_cast<void***>(chain.Get())=runtimeTable;
        CloseHandle(event); event = nullptr;
        DestroyWindow(window); window = nullptr;
        std::printf("Internal DX12 overlay (%s): Present, resize and GPU completion passed.\n", late ? "late attachment, hardware GPU" : "early attachment, WARP");
        return 0;
    } catch (const std::exception& error) {
        std::printf("Renderer smoke test FAILED: %s\n", error.what());
        meter::removeHooks();
        if (event) CloseHandle(event);
        if (window) DestroyWindow(window);
        return 1;
    }
}
