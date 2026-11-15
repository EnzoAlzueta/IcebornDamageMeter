#pragma once
#include <array>
#include <cstdint>
#include "language.hpp"
namespace meter {
constexpr int UnknownWeapon = 255;
struct WeaponStyle { const wchar_t* name; std::uint32_t color; };
// World native order, also documented by HunterPie's Weapon enum.
// Hues follow MHW guild-card icon references; swatches adjusted for HUD contrast.
inline constexpr std::array<WeaponStyle, 14> WeaponStyles{{
    {L"Great Sword", 0xd9535b}, {L"Sword & Shield", 0xd0b44f},
    {L"Dual Blades", 0x54a965}, {L"Long Sword", 0x2fc398},
    {L"Hammer", 0x596ad0}, {L"Hunting Horn", 0xe6a44e},
    {L"Lance", 0x50b6df}, {L"Gunlance", 0xe5a5b0},
    {L"Switch Axe", 0x7f9cc8}, {L"Charge Blade", 0xe47943},
    {L"Insect Glaive", 0xeddcaa}, {L"Bow", 0x48a6a4},
    {L"Heavy Bowgun", 0xc37bc8}, {L"Light Bowgun", 0xacc85b}
}};
inline bool validWeapon(int weapon) { return weapon >= 0 && weapon < 14; }
inline WeaponStyle weaponStyle(int weapon) {
    if(!validWeapon(weapon))return {tr(L"Unknown weapon"),0x748094};
    const auto& style=WeaponStyles[static_cast<size_t>(weapon)];
    return {tr(style.name),style.color};
}
}
