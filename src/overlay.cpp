#include "runtime.hpp"
#include <d3d11on12.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <array>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <optional>
#include "panel.hpp"
#include "view_state.hpp"
#include "weapon_icons.hpp"
#include "dx11_surface.hpp"
#include "process_com_ptr.hpp"
#include "damage_summary.hpp"
#include "game_icon_loader.hpp"
#include "game_weapon_icons.hpp"

namespace meter {
namespace {
using Microsoft::WRL::ComPtr;
std::mutex renderMutex;
ProcessComPtr<IDXGISwapChain> swapchain;
ProcessComPtr<IDXGISwapChain3> indexedSwapchain;
ProcessComPtr<ID3D11Device> gameDevice11;
// This renderer is process-owned; never destroy its driver objects under loader lock.
Dx11Surface& dx11 = *new Dx11Surface;
ProcessComPtr<ID3D12CommandQueue> queue;
ProcessComPtr<ID3D12Device> device12;
ProcessComPtr<ID3D11Device> device11;
ProcessComPtr<ID3D11DeviceContext> context11;
ProcessComPtr<ID3D11On12Device> on12;
ProcessComPtr<ID2D1Factory1> factory2d;
ProcessComPtr<ID2D1Device> device2d;
ProcessComPtr<ID2D1DeviceContext> context2d;
ProcessComPtr<IDWriteFactory> writeFactory;
ProcessComPtr<IDWriteTextFormat> font;
ProcessComPtr<IDWriteInlineObject> ellipsis;
ProcessComPtr<ID2D1SolidColorBrush> brush;
ProcessComPtr<ID3D12Fence> fence;
std::array<ProcessComPtr<ID3D11Resource>, 8> wrapped;
std::array<ProcessComPtr<ID2D1Bitmap1>, 8> targets;
UINT bufferCount = 0;
UINT64 fenceValue = 0;
HANDLE fenceEvent = nullptr;
HWND window = nullptr;
bool ready = false, disabled = false, resizing = false, visible = false, preview = false, settingsLoaded = false;
int corner = 3;
bool legacyPlacement = true;
#ifdef METER_DIAGNOSTIC
bool diagnosticAllWeapons = false;
bool diagnosticZeroDamage = false;
bool diagnosticFocused = true;
#endif
float scale = 1;
ULONGLONG lastPoll = 0;
Meter model;
Panel panel;
PanelView presentation;
std::uint64_t archivedSerial=0, activeStamp=0;
bool sceneDirty=true, refreshRequested=true;
#ifdef METER_DIAGNOSTIC
unsigned cacheBuilds=0;
#endif
ProcessComPtr<ID2D1CommandList> panelCommands;
struct PendingAction { Action action; int data; };
std::optional<PendingAction> pendingAction;
Visibility visibility;
Placement placement;
Rect panelRect, badgeRect;
float screenWidth = 0, screenHeight = 0;
ProcessComPtr<ID2D1PathGeometry> unknownWeaponIcon;
ProcessComPtr<ID2D1Bitmap1> gameWeaponAtlas;
std::wstring iniPath;
struct Row {
    std::wstring name, value;
    float fraction = 0;
    bool local = false;
    int weapon = UnknownWeapon;
    bool colored = true;
    std::wstring subtitle;
    ProcessComPtr<IDWriteTextLayout> nameLayout, valueLayout;
    ProcessComPtr<IDWriteTextLayout> subtitleLayout;
};
std::array<Row, 16> rows;
size_t rowCount = 0;
std::wstring heading = tr(L"DAMAGE · WAITING FOR QUEST"), footer, note;
ProcessComPtr<IDWriteTextLayout> headingLayout, footerLayout, noteLayout;
ProcessComPtr<IDWriteTextLayout> badgeLayout, closeLayout;
ProcessComPtr<IDWriteTextLayout> columnsLayout, comparisonLayout;
std::array<ProcessComPtr<IDWriteTextLayout>,48> buttonLayouts;
std::array<bool, 256> keys{};

void layout(const std::wstring& value, float width, bool alignRight, ComPtr<IDWriteTextLayout>& target,bool fit=false) {
    target.Reset();
    if (!font || !writeFactory || value.empty()) return;
    font->SetTextAlignment(alignRight ? DWRITE_TEXT_ALIGNMENT_TRAILING : DWRITE_TEXT_ALIGNMENT_LEADING);
    font->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    const auto hr = writeFactory->CreateTextLayout(value.c_str(), static_cast<UINT32>(value.size()),
        font.Get(), (std::max)(1.0f,width), 20, &target);
    if (FAILED(hr)) log("CreateTextLayout failed: 0x%08lx", static_cast<unsigned long>(hr));
    else {
        // Measure only when rebuilding narrow live rows, never each frame.
        if(fit) {
            DWRITE_TEXT_METRICS metrics{};
            target->SetMaxWidth(10000);
            if(SUCCEEDED(target->GetMetrics(&metrics))&&metrics.widthIncludingTrailingWhitespace>width) {
                const float size=(std::max)(9.0f,font->GetFontSize()*width/metrics.widthIncludingTrailingWhitespace*0.98f);
                target->SetFontSize(size,DWRITE_TEXT_RANGE{0,static_cast<UINT32>(value.size())});
            }
            target->SetMaxWidth((std::max)(1.0f,width));
        }
        if(ellipsis) {
            const DWRITE_TRIMMING trimming{DWRITE_TRIMMING_GRANULARITY_CHARACTER,0,0};
            target->SetTrimming(&trimming,ellipsis.Get());
        }
    }
    if(SUCCEEDED(hr)) {
        target->SetLocaleName(languageTag(),DWRITE_TEXT_RANGE{0,static_cast<UINT32>(value.size())});
        const bool rtl=currentLanguage()==Language::Arabic;
        target->SetReadingDirection(rtl?DWRITE_READING_DIRECTION_RIGHT_TO_LEFT:DWRITE_READING_DIRECTION_LEFT_TO_RIGHT);
        target->SetTextAlignment(alignRight?(rtl?DWRITE_TEXT_ALIGNMENT_LEADING:DWRITE_TEXT_ALIGNMENT_TRAILING):
            (rtl?DWRITE_TEXT_ALIGNMENT_TRAILING:DWRITE_TEXT_ALIGNMENT_LEADING));
    }
}
void refreshLayouts() {
    const float width=static_cast<float>(panel.config.width);
    layout(heading, width-(presentation.compact?50:76), false, headingLayout);
    layout(footer, width-24, false, footerLayout);
    layout(note, width-24, false, noteLayout);
    const auto geometry=rowGeometry(presentation,width);
    layout(presentation.columns,presentation.narrow?width-36:geometry.valueWidth,true,columnsLayout);
    layout(presentation.comparison,width-24,false,comparisonLayout);
    layout(L"DM", 28, false, badgeLayout);
    layout(L"×", 16, false, closeLayout);
    for (size_t i = 0; i < rowCount; ++i) {
        layout(rows[i].name, geometry.nameWidth, false, rows[i].nameLayout);
        layout(rows[i].value, geometry.valueWidth, true, rows[i].valueLayout,presentation.compact&&width<360);
        layout(rows[i].subtitle,geometry.subtitleWidth,false,rows[i].subtitleLayout);
    }
    for(size_t i=0;i<presentation.buttonCount;++i) layout(presentation.buttons[i].text,presentation.buttons[i].rect.width-8,false,buttonLayouts[i]);
    sceneDirty=true;
}

bool pressed(int vk) {
    const bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
    const bool edge = down && !keys[static_cast<size_t>(vk)];
    keys[static_cast<size_t>(vk)] = down;
    return edge;
}
void settings() {
    if (settingsLoaded) return;
    iniPath = folder + L"iceborne_damage_meter.ini";
    corner = (std::min)(3, static_cast<int>(GetPrivateProfileIntW(L"Overlay", L"Corner", 3, iniPath.c_str())));
    scale = static_cast<float>(GetPrivateProfileIntW(L"Overlay", L"ScalePercent", 100, iniPath.c_str()));
    scale = std::clamp(scale, 40.0f, 175.0f) / 100.0f;
    auto& c=panel.config;c.scale=static_cast<int>(scale*100);
    const auto read=[&](const wchar_t* key,int fallback) {return static_cast<int>(GetPrivateProfileIntW(L"Overlay",key,fallback,iniPath.c_str()));};
    c.opacity=read(L"Opacity",92);c.width=read(L"Width",460);c.maxRows=read(L"MaxRows",4);
    c.damage=read(L"ShowDamage",1)!=0;c.share=read(L"ShowPercent",1)!=0;c.dps=read(L"ShowDps",1)!=0;
    c.time=read(L"ShowParticipation",1)!=0;c.comparison=read(L"ShowComparison",1)!=0;
    c.showBadge=read(L"ShowBadge",1)!=0;c.autoShow=read(L"AutoShow",1)!=0;
    visibility.automatic(c.autoShow);
    c.showKey=read(L"ShowKey",0x77);c.historyKey=read(L"HistoryKey",0x76);
    c.settingsKey=read(L"SettingsKey",0x75);c.moveKey=read(L"MoveKey",0x78);c.normalize();
    const int savedX = static_cast<int>(GetPrivateProfileIntW(L"Overlay", L"PositionX", -1, iniPath.c_str()));
    const int savedY = static_cast<int>(GetPrivateProfileIntW(L"Overlay", L"PositionY", -1, iniPath.c_str()));
    if (savedX >= 0 && savedY >= 0) {
        placement.load(savedX / 10000.0f, savedY / 10000.0f);
        legacyPlacement = false;
    }
    // Ignore legacy Visible=1. AutoShow selects quest-driven or hotkey-only startup.
    settingsLoaded = true;
}
void saveSettings() {
    const auto& c=panel.config;
    const ProfileSettings values{{
        {L"PositionX",static_cast<int>(placement.x()*10000)},
        {L"PositionY",static_cast<int>(placement.y()*10000)},
        {L"ScalePercent",c.scale},{L"Opacity",c.opacity},{L"Width",c.width},{L"MaxRows",c.maxRows},
        {L"ShowDamage",c.damage},{L"ShowPercent",c.share},{L"ShowDps",c.dps},{L"ShowParticipation",c.time},
        {L"ShowComparison",c.comparison},{L"ShowKey",c.showKey},{L"HistoryKey",c.historyKey},
        {L"SettingsKey",c.settingsKey},{L"MoveKey",c.moveKey},
        {L"ShowBadge",c.showBadge},{L"AutoShow",c.autoShow}
    }};
    saveProfileAsync(values);
}
#include "panel_overlay.inl"
bool checked(HRESULT result, const char* operation) {
    if (SUCCEEDED(result)) return true;
    log("%s failed: 0x%08lx. Overlay disabled until restart.", operation, static_cast<unsigned long>(result));
    disabled = true;
    return false;
}
bool waitGpu() {
    if (!queue || !fence || !fenceEvent) return true;
    const auto value = ++fenceValue;
    if (FAILED(queue->Signal(fence.Get(), value))) return false;
    if (fence->GetCompletedValue() >= value) return true;
    if (FAILED(fence->SetEventOnCompletion(value, fenceEvent))) return false;
    return WaitForSingleObject(fenceEvent, 3000) == WAIT_OBJECT_0;
}
void releaseTargets() {
    if (context2d) context2d->SetTarget(nullptr);
    if (context11) { context11->ClearState(); context11->Flush(); }
    if (!waitGpu()) { log("GPU wait failed during resize; disabling overlay."); disabled = true; }
    for (auto& t : targets) t.Reset();
    for (auto& w : wrapped) w.Reset();
    if(gameDevice11)dx11.releaseTarget();
    // D3D11 deferred destruction must be flushed before DXGI resizes buffers.
    if (context11) context11->Flush();
    bufferCount = 0;
}
bool createTargets() {
    DXGI_SWAP_CHAIN_DESC desc{};
    if (!checked(swapchain->GetDesc(&desc), "GetDesc")) return false;
    if (desc.BufferCount > targets.size() || desc.BufferCount == 0) { disabled = true; log("Unsupported buffer count."); return false; }
    if (desc.BufferDesc.Format != DXGI_FORMAT_R8G8B8A8_UNORM && desc.BufferDesc.Format != DXGI_FORMAT_B8G8R8A8_UNORM &&
        !(gameDevice11&&(desc.BufferDesc.Format==DXGI_FORMAT_R8G8B8A8_UNORM_SRGB||desc.BufferDesc.Format==DXGI_FORMAT_B8G8R8A8_UNORM_SRGB))) {
        log("Unsupported render format %u. MVP supports SDR 8-bit.", desc.BufferDesc.Format);
        disabled = true; return false;
    }
    window = desc.OutputWindow;
    if(gameDevice11) {
        if(!checked(dx11.createTarget(swapchain.Get(),context2d.Get(),&targets[0]),"DX11 shared UI target"))return false;
        bufferCount=1;return true;
    }
    for (UINT i = 0; i < desc.BufferCount; ++i) {
        ComPtr<ID3D12Resource> resource;
        if (!checked(swapchain->GetBuffer(i, IID_PPV_ARGS(&resource)), "GetBuffer")) return false;
        D3D11_RESOURCE_FLAGS flags{};
        flags.BindFlags = D3D11_BIND_RENDER_TARGET;
        // The game has already transitioned its buffer to PRESENT at this point.
        if (!checked(on12->CreateWrappedResource(resource.Get(), &flags, D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&wrapped[i])), "CreateWrappedResource")) return false;
        ComPtr<IDXGISurface> surface;
        if (!checked(wrapped[i].As(&surface), "IDXGISurface")) return false;
        const auto properties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(desc.BufferDesc.Format, D2D1_ALPHA_MODE_IGNORE), 96, 96);
        if (!checked(context2d->CreateBitmapFromDxgiSurface(surface.Get(), &properties, &targets[i]), "CreateBitmapFromDxgiSurface")) return false;
    }
    bufferCount = desc.BufferCount;
    return true;
}
bool initializeGraphics() {
    if(gameDevice11) {
        ComPtr<IDXGIDevice> info;ComPtr<IDXGIAdapter> adapter;DXGI_ADAPTER_DESC description{};
        if(SUCCEEDED(gameDevice11.As(&info))&&SUCCEEDED(info->GetAdapter(&adapter))&&SUCCEEDED(adapter->GetDesc(&description))) {
            char name[256]{};WideCharToMultiByte(CP_UTF8,0,description.Description,-1,name,sizeof(name),nullptr,nullptr);
            log("DX11 adapter: %s vendor=%04x device=%04x dedicatedMB=%llu sharedMB=%llu feature=%04x flags=%08x",
                name,description.VendorId,description.DeviceId,static_cast<unsigned long long>(description.DedicatedVideoMemory>>20),
                static_cast<unsigned long long>(description.SharedSystemMemory>>20),static_cast<unsigned>(gameDevice11->GetFeatureLevel()),gameDevice11->GetCreationFlags());
        }
        if(!checked(dx11.initialize(gameDevice11.Get(),&device11,&context11),"DX11 UI device"))return false;
    } else {
    if (!checked(queue->GetDevice(IID_PPV_ARGS(&device12)), "GetDevice")) return false;
    IUnknown* queues[] = {queue.Get()};
    if (!checked(D3D11On12CreateDevice(device12.Get(), D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
        queues, 1, 0, &device11, &context11, nullptr), "D3D11On12CreateDevice")) return false;
    if (!checked(device11.As(&on12), "ID3D11On12Device")) return false;
    }
    if (!checked(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), nullptr,
        reinterpret_cast<void**>(factory2d.GetAddressOf())), "D2D1CreateFactory")) return false;
    ComPtr<IDXGIDevice> dxgiDevice;
    if (!checked(device11.As(&dxgiDevice), "IDXGIDevice")) return false;
    if (!checked(factory2d->CreateDevice(dxgiDevice.Get(), &device2d), "D2D CreateDevice")) return false;
    if (!checked(device2d->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &context2d), "D2D CreateDeviceContext")) return false;
    if (!checked(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(writeFactory.GetAddressOf())), "DWriteCreateFactory")) return false;
    if (!checked(writeFactory->CreateTextFormat(L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 12, languageTag(), &font), "CreateTextFormat")) return false;
    if(!checked(writeFactory->CreateEllipsisTrimmingSign(font.Get(),&ellipsis),"Create ellipsis"))return false;
    font->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    refreshLayouts();
    if (!checked(context2d->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush), "CreateSolidColorBrush")) return false;
    unknownWeaponIcon=createWeaponIcon(factory2d.Get(),UnknownWeapon);
    if(!checked(loadGameWeaponIcons(context2d.Get(),&gameWeaponAtlas),"Load original game weapon icons"))return false;
    log("Original game weapon icons ready: 14 native atlas cells, cached GPU bitmap.");
    context2d->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    if(device12) {
    if (!checked(device12->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "CreateFence")) return false;
    fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!fenceEvent) { disabled = true; log("CreateEvent failed."); return false; }
    }
    log("Internal D2D/%s renderer ready.",gameDevice11?"DX11":"DX12");
    return createTargets();
}
void text(IDWriteTextLayout* value, float left, float top, UINT32 color=0xe9edf5) {
    if (!value) return;
    brush->SetColor(D2D1::ColorF(color));
    context2d->DrawTextLayout(D2D1::Point2F(left, top), value, brush.Get(), D2D1_DRAW_TEXT_OPTIONS_CLIP);
}
#include "panel_render.inl"
bool matches(void* raw) {
    if (!swapchain || !raw) return false;
    ComPtr<IDXGISwapChain> candidate;
    return SUCCEEDED(static_cast<IDXGISwapChain*>(raw)->QueryInterface(IID_PPV_ARGS(&candidate))) && candidate.Get() == swapchain.Get();
}
}
bool attachSwapchain(void* raw, IUnknown* rawQueue) {
    std::lock_guard<std::mutex> lock(renderMutex);
    ComPtr<IDXGISwapChain> candidate;
    if (FAILED(static_cast<IDXGISwapChain*>(raw)->QueryInterface(IID_PPV_ARGS(&candidate)))) return false;
    if (candidate.Get() == swapchain.Get()) return true;
    if (swapchain) { log("Additional swapchain ignored in MVP."); return false; }
    if (FAILED(rawQueue->QueryInterface(IID_PPV_ARGS(&gameDevice11)))) {
        if (FAILED(rawQueue->QueryInterface(IID_PPV_ARGS(&queue))) || FAILED(candidate.As(&indexedSwapchain))) return false;
    }
    swapchain = candidate;
    DXGI_SWAP_CHAIN_DESC desc{};
    if (SUCCEEDED(swapchain->GetDesc(&desc))) {
        window = desc.OutputWindow;
        if (!installInput(window)) log("Input initialization unavailable; F8 still supported.");
    }
    return true;
}
void drawOverlay(void* raw, UINT flags) {
    if (flags & DXGI_PRESENT_TEST) return;
    std::lock_guard<std::mutex> lock(renderMutex);
    if (!matches(raw) || resizing || disabled) return;
    poll();
    static bool fullyHidden=false;
    if(!visible&&!panel.config.showBadge){
        if(!fullyHidden)setInputHotspot(RECT{});
        fullyHidden=true;return;
    }
    fullyHidden=false;
#ifdef METER_DIAGNOSTIC
    if(!diagnosticFocused)return;
#else
    if(!window||GetForegroundWindow()!=window||IsIconic(window))return;
#endif
    if (!ready) { ready = initializeGraphics(); if (!ready) return; }
    const UINT index = indexedSwapchain?indexedSwapchain->GetCurrentBackBufferIndex():0;
    if (index < bufferCount && targets[index]) paint(index);
}
void beforeResize(void* raw) {
    std::lock_guard<std::mutex> lock(renderMutex);
    if (!matches(raw)) return;
    resizing = true;
    releaseTargets();
}
void observePresentResult(void* raw,HRESULT result) {
    std::lock_guard<std::mutex> lock(renderMutex);
    static bool reported=false;
    if(!matches(raw)||!FAILED(result)||reported)return;
    reported=true;
    const auto reason=gameDevice11?gameDevice11->GetDeviceRemovedReason():device12?device12->GetDeviceRemovedReason():S_OK;
    log("Game Present failed: hr=0x%08lx deviceReason=0x%08lx api=%s; disabling meter rendering. Not proof of root cause.",
        static_cast<unsigned long>(result),static_cast<unsigned long>(reason),gameDevice11?"DX11":"DX12");
    disabled=true;
}
void afterResize(void* raw) {
    std::lock_guard<std::mutex> lock(renderMutex);
    if (!matches(raw)) return;
    if (ready && !disabled) createTargets();
    resizing = false;
}
void shutdownOverlay() {
    std::lock_guard<std::mutex> lock(renderMutex);
    releaseTargets();
    if (fenceEvent) CloseHandle(fenceEvent);
    fenceEvent = nullptr;
}
#ifdef METER_DIAGNOSTIC
void diagnosticPanelSize(int width,int percent) {
    std::lock_guard<std::mutex> lock(renderMutex);
    settings();panel.config.width=width;panel.config.scale=percent;panel.config.normalize();
    refreshRequested=true;
}
void diagnosticPreview(bool allWeapons, bool zeroDamage) {
    std::lock_guard<std::mutex> lock(renderMutex);
    diagnosticAllWeapons = allWeapons;
    diagnosticZeroDamage = zeroDamage;
    settings(); preview = true; visible = true; refreshRows();
}
bool diagnosticReady() {
    std::lock_guard<std::mutex> lock(renderMutex);
    D2D1_RECT_F bounds{};
    if(!gameWeaponAtlas||!unknownWeaponIcon||FAILED(unknownWeaponIcon->GetBounds(nullptr,&bounds))||bounds.right<=bounds.left||bounds.bottom<=bounds.top)return false;
    return ready && !disabled && bufferCount != 0;
}
unsigned diagnosticCacheBuilds(){return cacheBuilds;}
void diagnosticQuest(int id,bool maximumCounters) {
    std::lock_guard<std::mutex> lock(renderMutex);
    settings();model=Meter{};
    Sample sample;sample.valid=sample.countersValid=true;sample.questId=id;sample.state=2;
    const wchar_t* names[]={L"孤鸟",L"柳贯一",L"梅巧莉",L"Hunter"};
    for(size_t i=0;i<4;++i)sample.players[i]={i+1,names[i],0,i==2,true,static_cast<int>(i)};
    const auto now=GetTickCount64();
    model.update(sample,now-(maximumCounters?1000:490000));
    const unsigned values[]={30242,13115,0,1000};
    for(size_t i=0;i<4;++i)sample.players[i].damage=maximumCounters?100000000:values[i];
    model.update(sample,now);
    sample.state=3;model.update(sample,now);model.update(sample,now+3000);
    preview=false;panel.page=PanelPage::Current;
    if(!visibility.visible())visibility.toggle();
    visible=true;refreshRequested=true;refreshRows();
}
bool diagnosticCompactValuesFit() {
    std::lock_guard<std::mutex> lock(renderMutex);
    if(!presentation.compact||presentation.narrow||presentation.rowHeight!=25||!rowCount)return false;
    const auto geometry=rowGeometry(presentation,static_cast<float>(panel.config.width));
    for(size_t i=0;i<rowCount;++i) {
        if(!rows[i].valueLayout)return false;
        DWRITE_TEXT_METRICS metrics{};
        rows[i].valueLayout->SetMaxWidth(10000);
        const auto hr=rows[i].valueLayout->GetMetrics(&metrics);
        rows[i].valueLayout->SetMaxWidth(geometry.valueWidth);
        if(FAILED(hr)||metrics.widthIncludingTrailingWhitespace>geometry.valueWidth+0.5f)return false;
    }
    return true;
}
unsigned diagnosticDx11Composites(){return dx11.compositeCount();}
unsigned diagnosticDx11Rasterizations(){return dx11.rasterCount();}
void diagnosticFocus(bool focused){diagnosticFocused=focused;}
void diagnosticHidden(bool hidden){
    std::lock_guard<std::mutex> lock(renderMutex);
    settings();panel.config.showBadge=false;panel.config.autoShow=false;
    visibility.automatic(false);preview=false;setEditing(false);
    if(hidden)visibility.hide();else if(!visibility.visible())visibility.toggle();
    visible=visibility.visible();refreshRequested=true;
}
void diagnosticPanelPage(int page) {
    std::lock_guard<std::mutex> lock(renderMutex);
    const auto now=utcMilliseconds();
    for(int i=2;i>=0;--i) {
        HuntRecord r;r.timestamp=now-static_cast<std::uint64_t>(i+1)*3600000;r.questId=502;
        r.durationMs=504000;r.partySize=4;r.outcome=3;
        const wchar_t* names[]={tr(L"You"),tr(L"Hunter 2"),tr(L"Hunter 3"),tr(L"Hunter 4")};
        const int types[]={0,3,11,12};const unsigned damages[]={24500,21000,14000,10500};
        for(size_t j=0;j<4;++j) {
            r.players[j]={j+1,names[j],damages[j]-i*500,j==0,true,types[j]};
            r.players[j].participationMs=r.durationMs;
        }
        panel.history.add(r);
    }
    preview=false;
    panel.page=page==1?PanelPage::Settings:page==2?PanelPage::History:page==4?PanelPage::Menu:PanelPage::Record;
    if(!visibility.visible())visibility.toggle();
    visible=true;refreshRequested=true;refreshRows();
}
#endif
}
