#include "game_icon_loader.hpp"
#include "game_weapon_icons.hpp"
#include <wincodec.h>
#include <wrl/client.h>
namespace meter {
HRESULT loadGameWeaponIcons(ID2D1DeviceContext* context,ID2D1Bitmap1** result) {
    if(!context||!result)return E_POINTER;
    *result=nullptr;
    // Balance only our own initialization, never change an existing apartment.
    struct Apartment {
        HRESULT status=CoInitializeEx(nullptr,COINIT_MULTITHREADED);
        ~Apartment(){if(SUCCEEDED(status))CoUninitialize();}
    } apartment;
    if(FAILED(apartment.status)&&apartment.status!=RPC_E_CHANGED_MODE)return apartment.status;
    using Microsoft::WRL::ComPtr;
    ComPtr<IWICImagingFactory> factory;
    HRESULT hr=CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&factory));
    if(FAILED(hr))return hr;
    ComPtr<IWICStream> stream;
    if(FAILED(hr=factory->CreateStream(&stream)))return hr;
    if(FAILED(hr=stream->InitializeFromMemory(const_cast<BYTE*>(GameWeaponIconsPng.data()),static_cast<DWORD>(GameWeaponIconsPng.size()))))return hr;
    ComPtr<IWICBitmapDecoder> decoder;
    if(FAILED(hr=factory->CreateDecoderFromStream(stream.Get(),nullptr,WICDecodeMetadataCacheOnLoad,&decoder)))return hr;
    ComPtr<IWICBitmapFrameDecode> frame;
    if(FAILED(hr=decoder->GetFrame(0,&frame)))return hr;
    UINT width=0,height=0;
    if(FAILED(hr=frame->GetSize(&width,&height)))return hr;
    if(width!=896||height!=64)return E_INVALIDARG;
    ComPtr<IWICFormatConverter> converter;
    if(FAILED(hr=factory->CreateFormatConverter(&converter)))return hr;
    if(FAILED(hr=converter->Initialize(frame.Get(),GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone,nullptr,0,WICBitmapPaletteTypeCustom)))return hr;
    return context->CreateBitmapFromWicBitmap(converter.Get(),nullptr,result);
}
}
