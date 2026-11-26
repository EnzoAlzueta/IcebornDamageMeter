#include "../src/runtime.hpp"
#include "../src/language.hpp"
#include "../dependencies/preview-dev.1/iceborn_preview.hpp"
#include <d3d11_1.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#include <cmath>
#include "external_overlay.hpp"
using Microsoft::WRL::ComPtr;
extern "C" int IcebornDrawMeterPreviewV1(void*,const IcebornPreviewArea*);
extern "C" int IcebornDrawMeterSettingPreview(void*,const IcebornPreviewArea*,int,int);
namespace meter {
void diagnosticPreview(bool,bool);
bool diagnosticReady();
unsigned diagnosticCacheBuilds();
unsigned diagnosticDx11Composites();
unsigned diagnosticDx11Rasterizations();
void diagnosticFocus(bool);
void diagnosticHidden(bool);
void diagnosticPanelPage(int);
void diagnosticPanelSize(int,int);
void diagnosticRecapture(void*,IUnknown*);
void diagnosticQuest(int,bool);
bool diagnosticCompactValuesFit();
void diagnosticQueueSample(Sample);
std::uint64_t diagnosticTrackedDamage();
}
static void require(HRESULT result,const char* operation) {
    if(FAILED(result)){std::printf("%s: 0x%08lx\n",operation,static_cast<unsigned long>(result));throw std::runtime_error(operation);}
}
static bool renderedPixels(ID3D11Device* device,ID3D11DeviceContext* context,ID3D11Texture2D* back,const char* fileName) {
    D3D11_TEXTURE2D_DESC d{};back->GetDesc(&d);
    d.Usage=D3D11_USAGE_STAGING;d.BindFlags=0;d.CPUAccessFlags=D3D11_CPU_ACCESS_READ;d.MiscFlags=0;
    ComPtr<ID3D11Texture2D> staging;require(device->CreateTexture2D(&d,nullptr,&staging),"Readback texture");
    context->CopyResource(staging.Get(),back);
    D3D11_MAPPED_SUBRESOURCE map{};require(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&map),"Readback map");
    unsigned changed=0;std::vector<BYTE> bitmap(static_cast<size_t>(d.Width)*d.Height*4);
    for(UINT y=0;y<d.Height;++y)for(UINT x=0;x<d.Width;++x) {
        const auto pixel=static_cast<const BYTE*>(map.pData)+static_cast<size_t>(y)*map.RowPitch+x*4;
        if(std::abs(static_cast<int>(pixel[0])-13)>8||std::abs(static_cast<int>(pixel[1])-26)>8||std::abs(static_cast<int>(pixel[2])-38)>8)++changed;
        const auto at=(static_cast<size_t>(y)*d.Width+x)*4;
        bitmap[at]=pixel[2];bitmap[at+1]=pixel[1];bitmap[at+2]=pixel[0];bitmap[at+3]=255;
    }
    context->Unmap(staging.Get(),0);
    if(changed>300&&fileName) {
        BITMAPFILEHEADER f{};f.bfType=0x4d42;f.bfOffBits=sizeof(f)+sizeof(BITMAPINFOHEADER);f.bfSize=f.bfOffBits+static_cast<DWORD>(bitmap.size());
        BITMAPINFOHEADER info{};info.biSize=sizeof(info);info.biWidth=d.Width;info.biHeight=-static_cast<LONG>(d.Height);info.biPlanes=1;info.biBitCount=32;
        FILE* output=nullptr;fopen_s(&output,fileName,"wb");if(!output)throw std::runtime_error("Screenshot");
        fwrite(&f,sizeof(f),1,output);fwrite(&info,sizeof(info),1,output);fwrite(bitmap.data(),1,bitmap.size(),output);fclose(output);
    }
    return changed>300;
}
static std::uint32_t pixelAt(ID3D11Device* device, ID3D11DeviceContext* context,
                             ID3D11Texture2D* back, UINT x, UINT y) {
    D3D11_TEXTURE2D_DESC d{};
    back->GetDesc(&d);
    d.Width=d.Height=1;
    d.Usage=D3D11_USAGE_STAGING;
    d.BindFlags=0;
    d.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    ComPtr<ID3D11Texture2D> staging;
    require(device->CreateTexture2D(&d,nullptr,&staging),"Readback pixel");
    const D3D11_BOX region{x,y,0,x+1,y+1,1};
    context->CopySubresourceRegion(staging.Get(),0,0,0,0,back,0,&region);
    D3D11_MAPPED_SUBRESOURCE map{};
    require(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&map),"Readback pixel map");
    const auto pixel=*static_cast<const std::uint32_t*>(map.pData);
    context->Unmap(staging.Get(),0);
    return pixel;
}
static void checkPreviewEdges(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* chain) {
    meter::diagnosticPanelSize(260,100);
    ComPtr<ID3D11Texture2D> back;
    require(chain->GetBuffer(0,IID_PPV_ARGS(&back)),"Preview backbuffer");
    ComPtr<ID3D11RenderTargetView> output;
    require(device->CreateRenderTargetView(back.Get(),nullptr,&output),"Preview RTV");
    const float background[]{0.05f,0.1f,0.15f,1};
    context->ClearRenderTargetView(output.Get(),background);
    const auto original=pixelAt(device,context,back.Get(),225,210);
    const IcebornPreviewArea first{0,0,0,0,25,20};
    if(!IcebornDrawMeterPreviewV1(chain,&first) || pixelAt(device,context,back.Get(),225,210)==original)
        throw std::runtime_error("Suite preview did not paint the regression pixel");

    context->ClearRenderTargetView(output.Get(),background);
    // The menu displaces the preview to a fractional Y. Its bottom is 208.25.
    const IcebornPreviewArea second{190,214.25f,500,450,25,20};
    if(!IcebornDrawMeterPreviewV1(chain,&second) || pixelAt(device,context,back.Get(),225,210)!=original)
        throw std::runtime_error("Suite preview left a stale pixel outside the relocated panel");

    context->ClearRenderTargetView(output.Get(),background);
    const auto outside=pixelAt(device,context,back.Get(),425,135);
    if(!IcebornDrawMeterSettingPreview(chain,&first,2,75) ||
       pixelAt(device,context,back.Get(),425,135)!=outside)
        throw std::runtime_error("Suite preview did not apply the temporary scale");
    if(!IcebornDrawMeterPreviewV1(chain,&first) || pixelAt(device,context,back.Get(),425,135)==outside)
        throw std::runtime_error("Temporary Suite scale changed the configured value");
}
int main(int argc,char** argv) {
    const std::string mode=argc>1?argv[1]:"early";
    const bool externalBefore=mode.find("external-before")!=std::string::npos;
    const bool externalAfter=mode.find("external-after")!=std::string::npos;
    const bool late=externalBefore||mode.find("late")!=std::string::npos;
    const bool flip=mode.find("flip")!=std::string::npos;
    const bool hardware=mode.find("hardware")!=std::string::npos;
    const bool settings=mode.find("settings")!=std::string::npos;
    const bool history=mode.find("history")!=std::string::npos;
    const bool narrow=mode.find("narrow")!=std::string::npos;
    const bool chinese=mode.find("zh-cn")!=std::string::npos;
    const bool maximumCounters=mode.find("maximum")!=std::string::npos;
    const bool tinyScale=mode=="small";
    const bool focusTest=mode=="focus";
    const bool hiddenTest=mode=="hidden";
    const bool settledTest=mode=="settled-cache";
    const bool presentErrorTest=mode=="present-errors";
    const bool localeJapanese=mode=="locale-ja",localeArabic=mode=="locale-ar";
    const bool bgra=mode.find("bgra")!=std::string::npos;
    const bool copied=mode.find("copied")!=std::string::npos;
    HWND window=nullptr;
    try {
        wchar_t directory[32768]{};GetCurrentDirectoryW(32768,directory);meter::folder=std::wstring(directory)+L"\\";
        WNDCLASSW wc{};wc.lpfnWndProc=DefWindowProcW;wc.hInstance=GetModuleHandleW(nullptr);wc.lpszClassName=L"MeterDX11Test";
        if(!RegisterClassW(&wc))throw std::runtime_error("RegisterClass");
        window=CreateWindowW(wc.lpszClassName,L"DX11 test (hidden)",WS_OVERLAPPEDWINDOW,0,0,800,600,nullptr,nullptr,wc.hInstance,nullptr);
        if(!window)throw std::runtime_error("CreateWindow");
        if(!late&&!meter::installHooks())throw std::runtime_error("Early hooks");
        ComPtr<ID3D11Device> device;ComPtr<ID3D11DeviceContext> context;
        require(D3D11CreateDevice(nullptr,hardware?D3D_DRIVER_TYPE_HARDWARE:D3D_DRIVER_TYPE_WARP,nullptr,
            bgra?D3D11_CREATE_DEVICE_BGRA_SUPPORT:0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context),"DX11 device");
        ComPtr<IDXGIDevice> dxgi;require(device.As(&dxgi),"DXGI device");
        ComPtr<IDXGIAdapter> adapter;require(dxgi->GetAdapter(&adapter),"Adapter");
        ComPtr<IDXGIFactory2> factory;require(adapter->GetParent(IID_PPV_ARGS(&factory)),"Factory");
        ComPtr<IDXGISwapChain> chain;
        if(flip) {
            DXGI_SWAP_CHAIN_DESC1 d{};d.Width=800;d.Height=600;d.Format=DXGI_FORMAT_R8G8B8A8_UNORM;d.SampleDesc.Count=1;
            d.BufferCount=2;d.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;d.SwapEffect=DXGI_SWAP_EFFECT_FLIP_DISCARD;
            ComPtr<IDXGISwapChain1> modern;require(factory->CreateSwapChainForHwnd(device.Get(),window,&d,nullptr,nullptr,&modern),"Flip chain");
            require(modern.As(&chain),"Base chain");
        } else {
            DXGI_SWAP_CHAIN_DESC d{};d.BufferDesc.Width=800;d.BufferDesc.Height=600;d.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
            d.SampleDesc.Count=1;d.BufferCount=1;d.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;d.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
            d.OutputWindow=window;d.Windowed=TRUE;require(factory->CreateSwapChain(device.Get(),&d,&chain),"Legacy chain");
        }
        overlay_fixture::Overlay external;
        if(externalBefore)external.install(chain.Get());
        if(late&&!meter::installHooks())throw std::runtime_error("Late hooks");
        if(externalAfter){external.install(chain.Get());meter::diagnosticRecapture(chain.Get(),device.Get());}
        static std::array<void*,40> proxyTable{};
        void** runtimeTable=nullptr;
        if(copied) {
            auto slot=reinterpret_cast<void***>(chain.Get());runtimeTable=*slot;
            std::copy_n(runtimeTable,proxyTable.size(),proxyTable.begin());*slot=proxyTable.data();
            meter::diagnosticRecapture(chain.Get(),device.Get());
        }
        if(narrow||tinyScale)meter::diagnosticPanelSize(260,tinyScale?40:100);
        if(localeJapanese||localeArabic)meter::setLanguage(localeJapanese?meter::Language::Japanese:meter::Language::Arabic);
        meter::diagnosticPreview(false,false);if(settings||history||localeJapanese||localeArabic)meter::diagnosticPanelPage(history?2:1);
        if(chinese)meter::setLanguage(meter::Language::ChineseSimplified);
        if(chinese||maximumCounters||settledTest)meter::diagnosticQuest(1591,maximumCounters);
        D3D11_SAMPLER_DESC s{};s.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;s.AddressU=s.AddressV=s.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;s.MaxLOD=D3D11_FLOAT32_MAX;
        ComPtr<ID3D11SamplerState> sentinel;require(device->CreateSamplerState(&s,&sentinel),"Sentinel sampler");
        D3D11_BUFFER_DESC bd{};bd.ByteWidth=64;bd.Usage=D3D11_USAGE_DEFAULT;bd.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
        ComPtr<ID3D11Buffer> constant;require(device->CreateBuffer(&bd,nullptr,&constant),"Sentinel constant");
        bool pixelsBefore=false,pixelsAfter=false;
        if(hiddenTest)meter::diagnosticHidden(true);
        for(UINT frame=0;frame<36;++frame) {
            const bool hiddenFrame=hiddenTest&&(frame<2||(frame>=5&&frame<16));
            if(hiddenTest&&(frame==2||frame==5||frame==16))meter::diagnosticHidden(frame==5);
            const bool unfocusedFrame=focusTest&&frame>=5&&frame<10;
            meter::diagnosticFocus(!unfocusedFrame);
            const auto compositesBefore=meter::diagnosticDx11Composites();
            ComPtr<ID3D11Texture2D> back;require(chain->GetBuffer(0,IID_PPV_ARGS(&back)),"Backbuffer");
            ComPtr<ID3D11RenderTargetView> output;require(device->CreateRenderTargetView(back.Get(),nullptr,&output),"Game RTV");
            const float background[]={0.05f,0.1f,0.15f,1};context->ClearRenderTargetView(output.Get(),background);
            ID3D11RenderTargetView* rtv=output.Get();context->OMSetRenderTargets(1,&rtv,nullptr);
            const D3D11_VIEWPORT vp{3,5,321,213,0.2f,0.8f};context->RSSetViewports(1,&vp);
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
            ID3D11SamplerState* sampler=sentinel.Get();context->PSSetSamplers(5,1,&sampler);
            ID3D11Buffer* cb=constant.Get();context->VSSetConstantBuffers(2,1,&cb);context->CSSetConstantBuffers(3,1,&cb);
            context->SetPredication(nullptr,FALSE);
            if (settledTest) meter::diagnosticQueueSample(meter::Sample{});
            if(flip&&(frame&1)) {
                ComPtr<IDXGISwapChain1> modern;require(chain.As(&modern),"Present1 interface");
                const DXGI_PRESENT_PARAMETERS parameters{};require(modern->Present1(0,0,&parameters),"Present1");
            } else require(chain->Present(0,0),"Present");
            if(!hiddenFrame&&!meter::diagnosticReady())throw std::runtime_error("DX11 overlay did not initialize");
            D3D11_VIEWPORT after{};UINT count=1;context->RSGetViewports(&count,&after);
            D3D11_PRIMITIVE_TOPOLOGY topology{};context->IAGetPrimitiveTopology(&topology);
            ComPtr<ID3D11SamplerState> restored;context->PSGetSamplers(5,1,&restored);
            ComPtr<ID3D11Buffer> restoredVs,restoredCs;context->VSGetConstantBuffers(2,1,&restoredVs);context->CSGetConstantBuffers(3,1,&restoredCs);
            if(count!=1||after.TopLeftX!=vp.TopLeftX||after.Width!=vp.Width||after.MinDepth!=vp.MinDepth||
                topology!=D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP||restored.Get()!=sentinel.Get()||restoredVs.Get()!=constant.Get()||restoredCs.Get()!=constant.Get())
                throw std::runtime_error("Game pipeline state changed");
            const std::string file="preview-dx11-"+mode+".bmp";
            const bool pixels=renderedPixels(device.Get(),context.Get(),back.Get(),frame==10?file.c_str():nullptr);
            if(unfocusedFrame&&(pixels||meter::diagnosticDx11Composites()!=compositesBefore))throw std::runtime_error("Meter rendered while unfocused");
            if(hiddenFrame&&(pixels||meter::diagnosticDx11Composites()!=compositesBefore))throw std::runtime_error("Hidden meter still rendered a panel or DM badge");
            if(hiddenTest&&frame==16&&!pixels)throw std::runtime_error("Hidden meter did not recover after visibility toggle");
            if(focusTest&&frame==10&&!pixels)throw std::runtime_error("Meter did not resume after focus restoration");
            if(frame<12)pixelsBefore|=pixels;else pixelsAfter|=pixels;
            context->ClearState();output.Reset();back.Reset();
            if(frame==11)require(chain->ResizeBuffers(flip?2:1,1024,768,DXGI_FORMAT_R8G8B8A8_UNORM,0),"Resize larger");
            if(frame==23)require(chain->ResizeBuffers(flip?2:1,800,600,DXGI_FORMAT_R8G8B8A8_UNORM,0),"Resize back");
        }
        if(!pixelsBefore||!pixelsAfter||meter::diagnosticDx11Composites()<3)throw std::runtime_error("No UI pixels/composites around resize");
        if((chinese||maximumCounters)&&!meter::diagnosticCompactValuesFit())throw std::runtime_error("Narrow compact values clipped or stacked");
        if(hiddenTest?meter::diagnosticCacheBuilds()<2:meter::diagnosticCacheBuilds()!=1)throw std::runtime_error("Unexpected UI cache rebuild count");
        if(meter::diagnosticDx11Rasterizations()<3||meter::diagnosticDx11Rasterizations()>=meter::diagnosticDx11Composites())
            throw std::runtime_error("DX11 static raster was not reused across frames/resizes");
        std::printf("DX11 rasterizations: %u; composites: %u (includes two resizes).\n",meter::diagnosticDx11Rasterizations(),meter::diagnosticDx11Composites());
        if (presentErrorTest) {
            meter::observePresentResult(chain.Get(), DXGI_ERROR_WAS_STILL_DRAWING);
            if (!meter::diagnosticReady()) throw std::runtime_error("A busy Present disabled drawing");
            meter::observePresentResult(chain.Get(), DXGI_ERROR_INVALID_CALL);
            if (!meter::diagnosticReady()) throw std::runtime_error("Present failure on a live device disabled drawing");
            meter::observePresentResult(chain.Get(), DXGI_ERROR_DEVICE_REMOVED);
            if (meter::diagnosticReady()) throw std::runtime_error("Device removal left drawing enabled");
            const auto composites = meter::diagnosticDx11Composites();
            meter::Sample sample;
            sample.valid = sample.countersValid = true;
            sample.state = 2;
            sample.questId = 1591;
            sample.players[0] = {1, L"Hunter", 350, true, true, 0};
            meter::diagnosticQueueSample(sample);
            meter::drawOverlay(chain.Get(), 0);
            if (meter::diagnosticTrackedDamage() != 350 || meter::diagnosticDx11Composites() != composites)
                throw std::runtime_error("Disabled drawing stopped sampling or still composited pixels");
        }
        if(mode=="early")checkPreviewEdges(device.Get(),context.Get(),chain.Get());
        meter::shutdownOverlay();meter::removeHooks();
        external.restore();
        if((externalBefore||externalAfter)&&!overlay_fixture::calls)throw std::runtime_error("External overlay bypassed");
        if(runtimeTable)*reinterpret_cast<void***>(chain.Get())=runtimeTable;
        DestroyWindow(window);window=nullptr;
        std::printf("DX11 %s: real pixels, state preservation, cached drawing and resize passed (%u composites).\n",mode.c_str(),meter::diagnosticDx11Composites());
        return 0;
    } catch(const std::exception& e) {
        std::printf("DX11 test failed: %s\n",e.what());meter::removeHooks();if(window)DestroyWindow(window);return 1;
    }
}
