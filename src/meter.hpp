#pragma once
#include <array>
#include <algorithm>
#include <cstdint>
#include <string>
#include "weapons.hpp"
namespace meter {
struct Player {
    std::uint64_t id = 0;
    std::wstring name;
    std::uint32_t damage = 0;
    bool local = false;
    bool present = false;
    int weapon = UnknownWeapon;
    std::uint64_t participationMs = 0;
    std::uint32_t baselineDamage = 0;
    bool changedWeapon = false;
    double dps() const { return participationMs ? 1000.0 * (damage-baselineDamage) / participationMs : 0.0; }
};
struct Sample {
    bool valid = false;
    bool countersValid = false;
    int questId = 0;
    int state = 0;
    bool expedition = false;
    int stage = 0;
    std::array<Player, 4> players{};
};
inline bool isExpeditionField(int questId,int state,int stage) {
    const bool openMap=(stage>=101&&stage<=105)||stage==108||stage==109;
    return openMap&&(state==0||(state==2&&questId<=0));
}
enum class Phase { Waiting, Live, Settling, Summary };
class Meter {
    std::array<Player, 16> players_{};
    Phase phase_ = Phase::Waiting;
    int questId_ = 0;
    std::uint64_t finishAt_ = 0;
    bool partial_ = false, seenValid_ = false, previousInQuest_ = false, dataAvailable_ = false;
    std::uint64_t startAt_ = 0, lastMerge_ = 0, serial_ = 0;
    unsigned peakParty_ = 0;
    int outcome_ = 0;
    bool hasMerge_ = false;
    bool expedition_ = false;
    void merge(const Sample& sample, std::uint64_t now, bool advanceTime) {
        if (!sample.countersValid) { dataAvailable_ = false; return; }
        dataAvailable_ = true;
        if (advanceTime && hasMerge_) for (auto& p : players_)
            if (p.present) p.participationMs += now-lastMerge_;
        lastMerge_ = now; hasMerge_ = true;
        for (auto& p : players_) p.present = false;
        unsigned partySize = 0;
        for (const auto& incoming : sample.players) {
            if (!incoming.id || incoming.name.empty()) continue;
            Player* target = nullptr;
            for (auto& p : players_)
                if (p.id == incoming.id && p.name == incoming.name) { target = &p; break; }
            if (!target) for (auto& p : players_) if (!p.id) { target = &p; break; }
            if (!target) continue;
            if (!target->id) { *target = incoming; target->baselineDamage = incoming.damage; target->participationMs = 0; }
            target->damage = (std::max)(target->damage, incoming.damage);
            target->local = incoming.local;
            target->present = true;
            if (validWeapon(incoming.weapon)) {
                if (validWeapon(target->weapon) && target->weapon != incoming.weapon) target->changedWeapon = true;
                target->weapon = incoming.weapon;
            }
            ++partySize;
        }
        peakParty_ = (std::max)(peakParty_, partySize);
    }
public:
    // Counters are absolute mission totals, never add repeated samples.
    // Departed players stay in the denominator and the final summary.
    void update(const Sample& sample, std::uint64_t now) {
        if (!sample.valid) { dataAvailable_ = false; return; }
        if(sample.expedition) {
            if(!expedition_) {
                players_={};questId_=0;phase_=Phase::Waiting;
                peakParty_=0;outcome_=0;startAt_=finishAt_=lastMerge_=0;
                hasMerge_=false;partial_=false;++serial_;
            }
            expedition_=true;previousInQuest_=false;dataAvailable_=false;seenValid_=true;
            return;
        }
        expedition_=false;
        const bool inQuest = sample.state == 2 && sample.questId > 0;
        if (inQuest && (phase_ == Phase::Waiting || phase_ == Phase::Summary ||
                        sample.questId != questId_ || !previousInQuest_)) {
            players_ = {};
            questId_ = sample.questId;
            finishAt_ = 0;
            startAt_ = now; lastMerge_ = now; hasMerge_ = false;
            peakParty_ = 0; outcome_ = 0; ++serial_;
            partial_ = !seenValid_;
            phase_ = Phase::Live;
        }
        if (phase_ == Phase::Live && inQuest) merge(sample, now, true);
        if (phase_ == Phase::Live && !inQuest) {
            phase_ = Phase::Settling;
            finishAt_ = now;
            outcome_ = sample.state;
            if (hasMerge_) for (auto& p : players_) if (p.present) p.participationMs += now-lastMerge_;
        }
        if (phase_ == Phase::Settling) {
            if (sample.questId == questId_ && sample.state >= 3 && sample.state <= 5) merge(sample, now, false);
            if (now - finishAt_ >= 2500) phase_ = Phase::Summary;
        }
        previousInQuest_ = inQuest;
        seenValid_ = true;
    }
    Phase phase() const { return phase_; }
    bool expedition() const { return expedition_; }
    bool partial() const { return partial_; }
    bool dataAvailable() const { return dataAvailable_; }
    int questId() const { return questId_; }
    std::uint64_t serial() const { return serial_; }
    unsigned peakParty() const { return peakParty_; }
    int outcome() const { return outcome_; }
    std::uint64_t durationMs(std::uint64_t now) const {
        return phase_ == Phase::Waiting ? 0 : ((phase_ == Phase::Live ? now : finishAt_)-startAt_);
    }
    std::uint64_t total() const {
        std::uint64_t n = 0;
        for (const auto& p : players_) n += p.damage;
        return n;
    }
    double percent(const Player& p) const {
        const auto n = total();
        return n ? 100.0 * p.damage / static_cast<double>(n) : 0.0;
    }
    std::array<Player, 16> ranked() const {
        auto result = players_;
        std::stable_sort(result.begin(), result.end(), [](const Player& a, const Player& b) { return a.damage > b.damage; });
        return result;
    }
};
}
