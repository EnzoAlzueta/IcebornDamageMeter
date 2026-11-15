#pragma once
#include <Windows.h>
#include <cstdint>
#include <array>
#include <string>
#include "native_chat.hpp"
namespace meter {
// Experimental, explicit opt-in for hash-verified profile 421810 / game 15.23.
// Field layout reference: eigeen/mhw-toolkit game_export.rs + game_util.rs.
// No internal function calls, instruction patches, channel changes or keyboard.
class NativeChatMemory {
    std::uintptr_t ui_=0;
    template<class T> static bool read(std::uintptr_t at,T& value) {
        SIZE_T count=0;return at>=0x10000&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),&value,sizeof(value),&count)&&count==sizeof(value);
    }
    static bool writable(std::uintptr_t at,size_t size) {
        MEMORY_BASIC_INFORMATION region{};
        if(!VirtualQuery(reinterpret_cast<void*>(at),&region,sizeof(region))||region.State!=MEM_COMMIT||
            (region.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
        const auto mode=region.Protect&0xff;
        if(mode!=PAGE_READWRITE&&mode!=PAGE_WRITECOPY)return false;
        const auto end=reinterpret_cast<std::uintptr_t>(region.BaseAddress)+region.RegionSize;
        return at<end&&size<=end-at;
    }
    static bool write(std::uintptr_t at,const void* bytes,size_t size) {
        SIZE_T count=0;return writable(at,size)&&WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),bytes,size,&count)&&count==size;
    }
public:
    explicit NativeChatMemory(std::uintptr_t ui):ui_(ui){}
    bool sameObject(const NativeChatMemory& other) const{return ui_&&ui_==other.ui_;}
    static NativeChatMemory resolve() {
        const auto base=reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
        std::uintptr_t root=0,ui=0;
        if(!read(base+0x051c4640,root)||!root||!read(root+0x13fd0,ui))ui=0;
        return NativeChatMemory(ui);
    }
    NativeChatState probe() const {
        // UGUIChat +0x160: text entry visible; +0x165: UTF-8[128].
        // The send flag lives in the same UI object at +0x325e.
        int visible=-1;unsigned char flag=255;std::uintptr_t vtable=0;
        MEMORY_BASIC_INFORMATION tableRegion{};
        if(ui_<0x10000||ui_>0x00007fffffff0000ULL||!read(ui_+0x28f8,vtable)||
            !VirtualQuery(reinterpret_cast<void*>(vtable),&tableRegion,sizeof(tableRegion))||
            tableRegion.AllocationBase!=GetModuleHandleW(nullptr)||(tableRegion.Protect&0xff)!=PAGE_READONLY||
            !read(ui_+0x28f8+0x160,visible)||!read(ui_+0x325e,flag)||
            visible!=0||flag>1||!writable(ui_+0x28f8+0x165,128)||!writable(ui_+0x325e,1))return NativeChatState::Unavailable;
        return flag?NativeChatState::Busy:NativeChatState::Ready;
    }
    bool submit(const std::string& message,const std::string& previousOwnMessage={}) const {
        if(message.empty()||message.size()>127||message.find('\0')!=std::string::npos||probe()!=NativeChatState::Ready)return false;
        // Do not replace an unsent user draft, even with text entry hidden.
        std::array<char,128> previous{};
        if(!read(ui_+0x28f8+0x165,previous))return false;
        if(previous[0]) {
            // Some game screens leave the last consumed message in the buffer.
            // Only replace our exact previous submission in the same UI object;
            // hidden drafts and edits remain protected. probe() requires flag=0.
            std::array<char,128> owned{};
            if(previousOwnMessage.empty()||previousOwnMessage.size()>127)return false;
            std::copy(previousOwnMessage.begin(),previousOwnMessage.end(),owned.begin());
            if(previous!=owned)return false;
        }
        std::array<char,128> text{};std::copy(message.begin(),message.end(),text.begin());
        const unsigned char flag=1;
        return write(ui_+0x28f8+0x165,text.data(),text.size())&&write(ui_+0x325e,&flag,1);
    }
};
}
