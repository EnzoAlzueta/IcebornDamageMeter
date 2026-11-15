#include "../src/runtime.hpp"
#include "../src/language.hpp"
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
namespace meter {
void diagnosticPreview(bool,bool);
bool diagnosticReady();
unsigned diagnosticCacheBuilds();
unsigned diagnosticDx11Composites();
unsigned diagnosticDx11Rasterizations();
void diagnosticFocus(bool);
void diagnosticPanelPage(int);
void diagnosticPanelSize(int,int);
void diagnosticRecapture(void*,IUnknown*);
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
    const bool tinyScale=mode=="small";
    const bool focusTest=mode=="focus";
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
        D3D11_SAMPLER_DESC s{};s.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;s.AddressU=s.AddressV=s.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;s.MaxLOD=D3D11_FLOAT32_MAX;
        ComPtr<ID3D11SamplerState> sentinel;require(device->CreateSamplerState(&s,&sentinel),"Sentinel sampler");
        D3D11_BUFFER_DESC bd{};bd.ByteWidth=64;bd.Usage=D3D11_USAGE_DEFAULT;bd.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
        ComPtr<ID3D11Buffer> constant;require(device->CreateBuffer(&bd,nullptr,&constant),"Sentinel constant");
        bool pixelsBefore=false,pixelsAfter=false;
        for(UINT frame=0;frame<36;++frame) {
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
            if(flip&&(frame&1)) {
                ComPtr<IDXGISwapChain1> modern;require(chain.As(&modern),"Present1 interface");
                const DXGI_PRESENT_PARAMETERS parameters{};require(modern->Present1(0,0,&parameters),"Present1");
            } else require(chain->Present(0,0),"Present");
            if(!meter::diagnosticReady())throw std::runtime_error("DX11 overlay did not initialize");
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
            if(focusTest&&frame==10&&!pixels)throw std::runtime_error("Meter did not resume after focus restoration");
            if(frame<12)pixelsBefore|=pixels;else pixelsAfter|=pixels;
            context->ClearState();output.Reset();back.Reset();
            if(frame==11)require(chain->ResizeBuffers(flip?2:1,1024,768,DXGI_FORMAT_R8G8B8A8_UNORM,0),"Resize larger");
            if(frame==23)require(chain->ResizeBuffers(flip?2:1,800,600,DXGI_FORMAT_R8G8B8A8_UNORM,0),"Resize back");
        }
        if(!pixelsBefore||!pixelsAfter||meter::diagnosticDx11Composites()<3)throw std::runtime_error("No UI pixels/composites around resize");
        if(meter::diagnosticCacheBuilds()!=1)throw std::runtime_error("Static UI cache rebuilt");
        if(meter::diagnosticDx11Rasterizations()<3||meter::diagnosticDx11Rasterizations()>=meter::diagnosticDx11Composites())
            throw std::runtime_error("DX11 static raster was not reused across frames/resizes");
        std::printf("DX11 rasterizations: %u; composites: %u (includes two resizes).\n",meter::diagnosticDx11Rasterizations(),meter::diagnosticDx11Composites());
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
