#pragma once
#include "meter.hpp"
#include <vector>
#include <ostream>
#include <istream>
namespace meter {
struct HuntRecord {
    std::uint64_t timestamp = 0, durationMs = 0;
    int questId = 0, outcome = 0;
    unsigned partySize = 0;
    bool partial = false;
    std::array<Player, 16> players{};
    std::uint64_t total() const { std::uint64_t n=0; for (const auto& p:players) n+=p.damage; return n; }
    const Player* local() const { for (const auto& p:players) if(p.id && p.local) return &p; return nullptr; }
    bool success() const { return outcome == 3 || outcome == 4; }
};
inline HuntRecord captureHunt(const Meter& model, std::uint64_t now, std::uint64_t timestamp) {
    HuntRecord r;
    r.timestamp=timestamp; r.durationMs=model.durationMs(now); r.questId=model.questId();
    r.outcome=model.outcome(); r.partySize=model.peakParty(); r.partial=model.partial(); r.players=model.ranked();
    return r;
}
class History {
    std::vector<HuntRecord> records_;
public:
    History() { records_.reserve(10); }
    void add(HuntRecord r) {
        if (records_.size()==10) records_.pop_back();
        records_.insert(records_.begin(), std::move(r));
    }
    size_t size() const { return records_.size(); }
    const HuntRecord* get(size_t i) const { return i<records_.size()?&records_[i]:nullptr; }
    const HuntRecord* comparable(const HuntRecord& current) const {
        // Procedural investigations share this generic ID, not their targets.
        if(current.questId==30000)return nullptr;
        const auto p=current.local();
        if(current.outcome && !current.success())return nullptr;
        if (!p || p->changedWeapon || p->baselineDamage || current.partial || !validWeapon(p->weapon)) return nullptr;
        for (const auto& r:records_) {
            if (current.timestamp && r.timestamp >= current.timestamp) continue;
            const auto old=r.local();
            if (!r.success() || r.partial || r.questId!=current.questId || r.partySize!=current.partySize || !old) continue;
            if (old->name==p->name && old->weapon==p->weapon && !old->changedWeapon && !old->baselineDamage && old->participationMs>=1000)
                return &r;
        }
        return nullptr;
    }
};
namespace history_codec {
template<class T> bool write(std::ostream& out,T value) { out.write(reinterpret_cast<const char*>(&value),sizeof(value)); return bool(out); }
template<class T> bool read(std::istream& in,T& value) { return bool(in.read(reinterpret_cast<char*>(&value),sizeof(value))); }
inline bool writeRecord(std::ostream& out,const HuntRecord& r) {
    write(out,r.timestamp); write(out,r.durationMs); write(out,static_cast<std::int32_t>(r.questId));
    write(out,static_cast<std::int32_t>(r.outcome)); write(out,static_cast<std::uint32_t>(r.partySize)); write(out,std::uint8_t(r.partial));
    for(const auto& p:r.players) {
        write(out,p.id); write(out,p.damage); write(out,p.baselineDamage); write(out,p.participationMs);
        write(out,static_cast<std::int32_t>(p.weapon));
        write(out,std::uint8_t((p.local?1:0)|(p.present?2:0)|(p.changedWeapon?4:0)));
        const auto length=static_cast<std::uint16_t>((std::min)(size_t(64),p.name.size())); write(out,length);
        for(size_t i=0;i<length;++i) write(out,static_cast<std::uint16_t>(p.name[i]));
    }
    return bool(out);
}
inline bool readRecord(std::istream& in,HuntRecord& r) {
    std::int32_t quest=0,outcome=0; std::uint32_t party=0; std::uint8_t partial=0;
    if(!read(in,r.timestamp)||!read(in,r.durationMs)||!read(in,quest)||!read(in,outcome)||!read(in,party)||!read(in,partial)) return false;
    if(quest<=0||quest>1000000||outcome<0||outcome>7||party>4||partial>1||r.durationMs>24*60*60*1000ULL) return false;
    r.questId=quest; r.outcome=outcome; r.partySize=party; r.partial=partial!=0;
    for(auto& p:r.players) {
        std::int32_t weapon=255; std::uint8_t flags=0; std::uint16_t length=0;
        if(!read(in,p.id)||!read(in,p.damage)||!read(in,p.baselineDamage)||!read(in,p.participationMs)||
            !read(in,weapon)||!read(in,flags)||!read(in,length)) return false;
        if(length>64||p.damage>100000000||p.baselineDamage>p.damage||p.participationMs>24*60*60*1000ULL||
            (!validWeapon(weapon)&&weapon!=255)||flags>7) return false;
        p.weapon=weapon; p.local=(flags&1)!=0; p.present=(flags&2)!=0; p.changedWeapon=(flags&4)!=0;
        p.name.clear(); p.name.reserve(length);
        if((p.id&&length==0)||(!p.id&&(length||p.damage||p.baselineDamage||p.participationMs||flags)))return false;
        for(unsigned i=0;i<length;++i) { std::uint16_t c=0; if(!read(in,c)||c<32) return false; p.name.push_back(static_cast<wchar_t>(c)); }
    }
    return true;
}
inline bool save(std::ostream& out,const History& history) {
    out.write("IDMHST03",8); write(out,static_cast<std::uint32_t>(history.size()));
    for(size_t i=0;i<history.size();++i) if(!writeRecord(out,*history.get(i))) return false;
    return bool(out);
}
inline bool load(std::istream& in,History& history) {
    char magic[8]{}; std::uint32_t count=0;
    if(!in.read(magic,8)||std::string(magic,8)!="IDMHST03"||!read(in,count)||count>10) return false;
    std::array<HuntRecord,10> records{};
    for(unsigned i=0;i<count;++i) if(!readRecord(in,records[i])) return false;
    if(in.peek()!=std::char_traits<char>::eof()) return false;
    History loaded;
    for(unsigned i=count;i>0;--i) loaded.add(std::move(records[i-1]));
    history=std::move(loaded); return true;
}
}
}
