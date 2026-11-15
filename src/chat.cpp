#include "runtime.hpp"
#include "share_chat.hpp"
#include "chat_input.hpp"
#include "native_chat_memory.hpp"
#include <optional>
#include <cstring>
namespace meter {
namespace {
ChatPlayback playback;
NativeChatPlayback nativePlayback;
std::optional<NativeChatMemory> nativeMemory;
std::optional<NativeChatMemory> previousNativeMemory;
std::string previousNativeMessage;
bool useNative=false;
HWND target=nullptr;
bool outcomeReady=false;
bool emit(const ChatStep& step) {
    if(step.operation==ChatOperation::Text) {
        std::array<INPUT,160> inputs{};size_t count=0;
        for(const auto c:step.text) {
            auto& down=inputs[count++];down.type=INPUT_KEYBOARD;
            down.ki.wScan=static_cast<WORD>(c);down.ki.dwFlags=KEYEVENTF_UNICODE;
            auto& up=inputs[count++];up=down;up.ki.dwFlags|=KEYEVENTF_KEYUP;
        }
        return SendInput(static_cast<UINT>(count),inputs.data(),sizeof(INPUT))==count;
    }
    INPUT input=chatKeyInput(step.key,step.operation==ChatOperation::KeyUp);
    return SendInput(1,&input,sizeof(input))==1;
}
}
bool beginDamageChat(HWND window,const HuntRecord& record,int openKey) {
    if(!window||GetForegroundWindow()!=window||!supported.load()||
        (GetAsyncKeyState(VK_CONTROL)&0x8000)||(GetAsyncKeyState(VK_MENU)&0x8000)||
        (GetAsyncKeyState(VK_SHIFT)&0x8000))return false;
    const auto lines=damageChatLines(record);
    if(damageChatBusy())return false;
    wchar_t transport[32]{};
    GetPrivateProfileStringW(L"Overlay",L"ChatTransport",L"keyboard",transport,32,(folder+L"iceborne_damage_meter.ini").c_str());
    useNative=_wcsicmp(transport,L"native")==0;
    if(useNative) {
        nativeMemory=NativeChatMemory::resolve();
        if(nativeMemory->probe()!=NativeChatState::Ready){log("Native chat unavailable/busy; no keyboard fallback or message sent.");return false;}
        std::vector<std::string> encoded;
        for(const auto& line:lines) {
            const auto count=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,line.data(),static_cast<int>(line.size()),nullptr,0,nullptr,nullptr);
            if(count<=0||count>127)return false;
            std::string bytes(static_cast<size_t>(count),'\0');
            if(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,line.data(),static_cast<int>(line.size()),bytes.data(),count,nullptr,nullptr)!=count)return false;
            encoded.push_back(std::move(bytes));
        }
        if(!nativePlayback.start(std::move(encoded),GetTickCount64()))return false;
    } else if(!playback.start(lines,GetTickCount64(),openKey))return false;
    outcomeReady=false;
    target=window;
    log("Manual chat share requested: quest=%d total=%llu lines=%zu transport=%s",record.questId,record.total(),lines.size(),useNative?"native (experimental)":"paced keyboard");
    return true;
}
void tickDamageChat() {
    if(useNative) {
        const auto before=nativePlayback.result();
        nativePlayback.tick(GetTickCount64(),target&&GetForegroundWindow()==target,
            (GetAsyncKeyState(VK_LBUTTON)&0x8000)!=0,(GetAsyncKeyState(VK_ESCAPE)&0x8000)!=0,
            []{return nativeMemory&&nativeMemory->sameObject(NativeChatMemory::resolve())?nativeMemory->probe():NativeChatState::Unavailable;},
            [](const std::string& message){
                if(!supported.load()||!nativeMemory||!nativeMemory->sameObject(NativeChatMemory::resolve()))return false;
                const auto owned=previousNativeMemory&&nativeMemory->sameObject(*previousNativeMemory)?previousNativeMessage:std::string{};
                if(!nativeMemory->submit(message,owned)){log("Native chat submit refused: buffer/draft or memory guard; no retry.");return false;}
                previousNativeMemory=nativeMemory;previousNativeMessage=message;return true;
            });
        if(before!=nativePlayback.result()) {
            outcomeReady=true;
            log("Native chat request ended: status=%d; NOT a delivery acknowledgement",static_cast<int>(nativePlayback.result()));
        }
        return;
    }
    const auto before=playback.result();
    // Ignore the Escape synthesized by our own final step: it has no purpose
    // beyond closing chat. Physical Escape is still honoured before that step.
    playback.tick(GetTickCount64(),target&&GetForegroundWindow()==target,
        (GetAsyncKeyState(VK_LBUTTON)&0x8000)!=0,
        !playback.closingChat()&&(GetAsyncKeyState(VK_ESCAPE)&0x8000)!=0,emit);
    if(before!=playback.result()) {
        outcomeReady=true;
        log("Manual chat input ended: status=%d; NOT a delivery acknowledgement",static_cast<int>(playback.result()));
    }
}
bool damageChatBusy(){return playback.busy()||nativePlayback.busy();}
bool takeDamageChatResult(ChatResult& result) {
    if(!outcomeReady)return false;
    result=useNative?nativePlayback.result():playback.result();outcomeReady=false;return true;
}
bool copyDamageSummary(HWND window,const HuntRecord& record) {
    const auto lines=damageSummaryLines(record);if(lines.empty())return false;
    std::wstring text;for(const auto& line:lines){if(!text.empty())text+=L"\r\n";text+=line;}
    const SIZE_T bytes=(text.size()+1)*sizeof(wchar_t);
    HGLOBAL memory=GlobalAlloc(GMEM_MOVEABLE,bytes);if(!memory)return false;
    void* destination=GlobalLock(memory);
    if(!destination){GlobalFree(memory);return false;}
    std::memcpy(destination,text.c_str(),bytes);GlobalUnlock(memory);
    if(!OpenClipboard(window)){GlobalFree(memory);return false;}
    const bool ok=EmptyClipboard()&&SetClipboardData(CF_UNICODETEXT,memory)!=nullptr;
    CloseClipboard();if(!ok)GlobalFree(memory);return ok;
}
}
