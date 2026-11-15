#pragma once
#include <Windows.h>
namespace meter {
inline INPUT chatKeyInput(int key,bool up) {
    INPUT input{};input.type=INPUT_KEYBOARD;
    const UINT scan=MapVirtualKeyW(static_cast<UINT>(key),MAPVK_VK_TO_VSC_EX);
    input.ki.wScan=static_cast<WORD>(scan&0xff);
    input.ki.dwFlags=KEYEVENTF_SCANCODE|(up?KEYEVENTF_KEYUP:0);
    // Some layouts map VK_INSERT to bare 0x52 (the keypad shares it). Force
    // the E0 flag so this remains the dedicated Insert key, never numpad zero.
    if(key==VK_INSERT||(scan&0xff00)==0xe000)input.ki.dwFlags|=KEYEVENTF_EXTENDEDKEY;
    return input;
}
}
