#include "runtime.hpp"
#include "version.hpp"
#include <bcrypt.h>
#include <array>
#include <cstring>
#include <vector>
namespace meter {
namespace {
std::uintptr_t base;
constexpr char TestedHash[] = IDM_GAME_SHA256;
template<class T> bool read(std::uintptr_t address, T& result) {
    SIZE_T copied = 0;
    return address >= 0x10000 && ReadProcessMemory(GetCurrentProcess(),
        reinterpret_cast<const void*>(address), &result, sizeof(T), &copied) && copied == sizeof(T);
}
std::uintptr_t chain(std::uintptr_t address, std::initializer_list<std::uintptr_t> offsets) {
    for (const auto offset : offsets) {
        std::uintptr_t p = 0;
        if (!read(address, p) || p < 0x10000 || p > 0x00007fffffffffffULL) return 0;
        address = p + offset;
    }
    return address;
}
std::wstring nameAt(std::uintptr_t address) {
    char bytes[32]{};
    if (!read(address, bytes)) return {};
    bytes[31] = 0;
    const int count = static_cast<int>(strnlen_s(bytes, 31));
    if (!count) return {};
    wchar_t wide[33]{};
    const int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes, count, wide, 32);
    if (!n) return {};
    for (int i = 0; i < n; ++i) if (wide[i] < 32) return {};
    return {wide, static_cast<size_t>(n)};
}
std::wstring localName() {
    const auto header = chain(base + 0x05013950, {0xa8});
    std::uint32_t slot = 0;
    std::uintptr_t saves = 0;
    if (!header || !read(header + 0x44, slot) || slot > 2 || !read(header, saves)) return {};
    return nameAt(saves + 0x26cc00ULL * slot + 0x50);
}
bool hashExecutable(char (&hex)[65]) {
    wchar_t path[32768]{};
    if (!GetModuleFileNameW(nullptr, path, 32768)) return false;
    HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    bool ok = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) >= 0;
    if (ok) ok = BCryptCreateHash(algorithm, &hash, nullptr, 0, nullptr, 0, 0) >= 0;
    std::vector<UCHAR> buffer(1024 * 1024);
    DWORD count = 0;
    while (ok) {
        if (!ReadFile(file, buffer.data(), static_cast<DWORD>(buffer.size()), &count, nullptr)) { ok = false; break; }
        if (!count) break;
        ok = BCryptHashData(hash, buffer.data(), count, 0) >= 0;
    }
    UCHAR digest[32]{};
    if (ok) ok = BCryptFinishHash(hash, digest, sizeof(digest), 0) >= 0;
    if (hash) BCryptDestroyHash(hash);
    if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0);
    CloseHandle(file);
    constexpr char digits[] = "0123456789abcdef";
    if (ok) for (int i = 0; i < 32; ++i) {
        hex[2*i] = digits[digest[i] >> 4]; hex[2*i+1] = digits[digest[i] & 15];
    }
    hex[64] = 0;
    return ok;
}
}
void initializeReader() {
    base = reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    char hash[65]{};
    if (!hashExecutable(hash)) { log("Cannot verify game executable. Damage reading disabled."); return; }
    supported = std::strcmp(hash, TestedHash) == 0;
    log("Game SHA256 %s; memory profile %s", hash, supported.load() ? "421810 enabled" : "unsupported (disabled)");
}
Sample readSample() {
    Sample s;
    if (!supported.load()) return s;
    const auto quest = chain(base + 0x0500ed30, {0});
    if (!quest || !read(quest + 0x4c, s.questId) || !read(quest + 0x54, s.state)) return s;
    if (s.questId < -1 || s.questId > 1000000 || s.state < 0 || s.state > 7) return s;
    if (s.questId == -1) s.questId = 0; // normal no-quest sentinel in town
    // Known read-only zone field in the verified 421810 profile.
    const auto zone=chain(base+0x0500eca0,{0xaed0});
    if(zone)read(zone,s.stage);
    s.expedition=isExpeditionField(s.questId,s.state,s.stage);
    s.valid = true;
    static int lastId=-2,lastState=-1,lastStage=-1;
    if(lastId!=s.questId||lastState!=s.state||lastStage!=s.stage) {
        log("Field context: quest=%d state=%d stage=%d expedition=%d",s.questId,s.state,s.stage,s.expedition);
        lastId=s.questId;lastState=s.state;lastStage=s.stage;
    }
    if(s.expedition)return s; // quest-award counters are not expedition totals
    if (s.state < 2 || s.state > 5 || s.questId <= 0) return s;
    const auto party = chain(base + 0x05013530, {0x1ab0});
    const auto damage = chain(base + 0x051c46b8, {0x258, 0x38, 0x450, 0x8, 0x48});
    if (!party || !damage) return s;
    const auto ownName = localName();
    int ownWeapon = UnknownWeapon;
    const auto weaponAddress = chain(base + 0x050139a0, {0x50, 0xc0, 0x8, 0x78, 0x2e8});
    std::uint8_t localType = 255;
    if (weaponAddress && read(weaponAddress, localType) && validWeapon(localType)) ownWeapon = localType;
    bool any = false;
    for (size_t i = 0; i < 4; ++i) {
        std::uintptr_t player = 0;
        // Party entries: nine int64 fields and four int32 fields.
        if (!read(party + i * 0x58, player)) return s;
        if (!player) continue;
        auto name = nameAt(player + 0x49);
        if (name.empty()) continue;
        std::int32_t value = 0;
        if (!read(damage + i * 0x2a0, value) || value < 0 || value > 100000000) return s;
        s.players[i] = {player, name, static_cast<std::uint32_t>(value), !ownName.empty() && ownName == name, true};
        std::uint8_t weapon = 255;
        if (s.players[i].local && validWeapon(ownWeapon)) s.players[i].weapon = ownWeapon;
        else if (read(player + 0x7c, weapon) && validWeapon(weapon)) s.players[i].weapon = weapon;
        any = true;
    }
    // Reject a sample taken across a quest transition or party-slot replacement.
    int endingState = -1, endingId = -1;
    if (!read(quest + 0x54, endingState) || !read(quest + 0x4c, endingId) ||
        endingState != s.state || endingId != s.questId) return Sample{};
    for (size_t i = 0; i < 4; ++i) if (s.players[i].id) {
        std::uintptr_t current = 0;
        if (!read(party + i * 0x58, current) || current != s.players[i].id) return Sample{};
    }
    s.countersValid = any;
    return s;
}
}
