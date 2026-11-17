#include "runtime.hpp"
#include "damage_summary.hpp"
#include <cstring>
namespace meter {
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
