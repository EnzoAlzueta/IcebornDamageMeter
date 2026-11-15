#pragma once
#include <Windows.h>
#include <Unknwn.h>
#include <atomic>
#include <mutex>
#include <string>
#include "meter.hpp"
#include "history.hpp"
namespace meter {
extern HMODULE module;
extern std::wstring folder;
extern std::atomic<bool> supported, stopping;
void log(const char* format, ...);
void initializeReader();
Sample readSample();
bool installHooks();
void removeHooks();
bool attachSwapchain(void* swapchain, IUnknown* queueOrDx11Device);
void drawOverlay(void* swapchain, UINT flags);
void observePresentResult(void* swapchain,HRESULT result);
void beforeResize(void* swapchain);
void afterResize(void* swapchain);
void shutdownOverlay();
// Shared slot registry, used by DirectInput and the game's cursor imports.
bool hookSlot(void** slot, void* replacement);
void* originalHook(void* object, size_t index);
bool installInput(HWND window);
void setEditing(bool enabled);
bool isEditing();
void setInputHotspot(const RECT& rect);
bool getEditPointer(POINT& clientPoint, bool& leftDown);
bool beginDamageChat(HWND window,const HuntRecord& record,int openKey);
void tickDamageChat();
bool damageChatBusy();
enum class ChatResult;
bool takeDamageChatResult(ChatResult& result);
bool copyDamageSummary(HWND window,const HuntRecord& record);
void startHistoryStore(const std::wstring& directory);
bool takeLoadedHistory(History& history);
void saveHistoryAsync(const History& history,const HuntRecord& last);
struct ProfileSetting {const wchar_t* key=nullptr;int value=0;};
void saveProfileAsync(const std::array<ProfileSetting,16>& values);
}
