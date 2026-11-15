#pragma once
#include "history.hpp"
#include "quests.hpp"
#include <vector>
#include <cstdio>

namespace meter {
inline bool canShareDamage(const Meter& model,bool preview) {
    return !preview&&model.questId()>0&&model.total()>0;
}
inline std::wstring chatText(const std::wstring& text,size_t limit) {
    if(!limit)return {};
    std::wstring clean;
    for(const auto c:text) {
        if(c>=32&&c!=127&&c!=L'\u2028'&&c!=L'\u2029')clean+=c;
        if(clean.size()==limit)break;
    }
    if(!clean.empty()&&clean.back()>=0xd800&&clean.back()<=0xdbff)clean.pop_back();
    return clean;
}
inline size_t chatUtf8Bytes(const std::wstring& text) {
    size_t bytes=0;
    for(size_t i=0;i<text.size();++i) {
        const auto c=text[i];
        if(c>=0xd800&&c<=0xdbff&&i+1<text.size()&&text[i+1]>=0xdc00&&text[i+1]<=0xdfff){bytes+=4;++i;}
        else bytes+=c<0x80?1:c<0x800?2:3;
    }
    return bytes;
}
inline std::wstring chatName(const std::wstring& name,size_t characters,size_t bytes) {
    auto result=chatText(name,characters);
    while(!result.empty()&&chatUtf8Bytes(result)>bytes) {
        result.pop_back();
        if(!result.empty()&&result.back()>=0xd800&&result.back()<=0xdbff)result.pop_back();
    }
    return result;
}
// Short UTF-16 lines stay below the game's text field limit. Values are a single
// snapshot of mission totals, including recorded players who already departed.
inline std::vector<std::wstring> damageSummaryLines(const HuntRecord& record) {
    if(record.questId<=0||!record.total())return {};
    std::vector<std::wstring> lines;
    lines.push_back(tr(L"[Damage] ")+chatText(questName(record.questId),36)+tr(L" | Total: ")+std::to_wstring(record.total()));
    const std::wstring prefix=tr(L"[Damage] ");
    std::wstring line=prefix;
    for(const auto& player:record.players)if(player.id) {
        wchar_t percent[24]{};
        swprintf_s(percent,L" (%.1f%%)",100.0*player.damage/record.total());
        const auto item=chatText(player.name,12)+L": "+std::to_wstring(player.damage)+percent;
        if(line.size()>prefix.size()&&line.size()+3+item.size()>80) {lines.push_back(line);line=prefix;}
        if(line.size()>prefix.size())line+=L" | ";
        line+=item;
    }
    if(line.size()>prefix.size())lines.push_back(line);
    return lines;
}
// Preserve full names whenever a single player's entry fits. Pack complete
// entries into bounded messages rather than reserving room for four worst cases.
inline std::vector<std::wstring> damageChatLines(const HuntRecord& record) {
    if(record.questId<=0||!record.total())return {};
    const std::wstring prefix=tr(L"[Damage] ");
    std::vector<std::wstring> lines;
    std::wstring line=prefix;
    for(const auto& player:record.players)if(player.id) {
        const auto value=L": "+std::to_wstring(player.damage);
        const auto item=chatName(player.name,80-prefix.size()-value.size(),127-chatUtf8Bytes(prefix)-chatUtf8Bytes(value))+value;
        if(line.size()>prefix.size()&&(line.size()+3+item.size()>80||chatUtf8Bytes(line)+3+chatUtf8Bytes(item)>127)){lines.push_back(line);line=prefix;}
        if(line.size()>prefix.size())line+=L" | ";
        line+=item;
    }
    if(line.size()>prefix.size())lines.push_back(line);
    return lines;
}
enum class ChatOperation { KeyDown,KeyUp,Text };
struct ChatStep {ChatOperation operation;int key=0;std::wstring text;unsigned wait=0;};
enum class ChatResult { Idle,Sending,Completed,Cancelled,Failed };
class ChatPlayback {
    std::vector<ChatStep> steps_;
    size_t step_=0;
    int held_=0;
    std::uint64_t due_=0,started_=0,cooldown_=0;
    ChatResult result_=ChatResult::Idle;
    void key(int code,unsigned after=300) {
        steps_.push_back({ChatOperation::KeyDown,code,{},100});
        steps_.push_back({ChatOperation::KeyUp,code,{},after});
    }
public:
    ChatResult result() const {return result_;}
    bool busy() const {return result_==ChatResult::Sending;}
    bool closingChat() const {return held_==0x1b;}
    bool start(const std::vector<std::wstring>& lines,std::uint64_t now,int openKey) {
        if(busy()||now<cooldown_||lines.empty()||lines.size()>17||(openKey!=0x2d&&openKey!=0x0d))return false;
        for(const auto& line:lines)if(line.empty()||line.size()>80||chatUtf8Bytes(line)>127||line!=chatText(line,80))return false;
        steps_.clear();step_=0;held_=0;due_=now+200;started_=now;cooldown_=now+15000;
        key(openKey,650);
        for(size_t i=0;i<lines.size();++i) {
            // Insert opens the log; Enter activates text entry. If Enter is the
            // configured open key, its first press already opens text entry.
            if(i||openKey!=0x0d)key(0x0d,500);
            // Give the game's per-frame input processing time between Unicode
            // characters. Keep UTF-16 surrogate pairs in the same input event.
            for(size_t pos=0;pos<lines[i].size();) {
                const size_t count=lines[i][pos]>=0xd800&&lines[i][pos]<=0xdbff&&pos+1<lines[i].size()?2:1;
                steps_.push_back({ChatOperation::Text,0,lines[i].substr(pos,count),45});pos+=count;
            }
            steps_.back().wait=450;
            key(0x0d,1500);
        }
        key(0x1b,100);
        result_=ChatResult::Sending;return true;
    }
    template<class Emit> void tick(std::uint64_t now,bool focused,bool mouseDown,bool escape,Emit emit) {
        if(!busy())return;
        const auto stop=[&](ChatResult why) {
            if(held_)emit(ChatStep{ChatOperation::KeyUp,held_,{},0});
            held_=0;steps_.clear();result_=why;
        };
        if(!focused||escape) {stop(ChatResult::Cancelled);return;}
        if(now-started_>120000) {stop(ChatResult::Failed);return;}
        if(now<due_)return;
        if(!step_&&mouseDown)return;
        if(step_==steps_.size()) {steps_.clear();result_=ChatResult::Completed;return;}
        const auto& step=steps_[step_];
        if(!emit(step)) {stop(ChatResult::Failed);return;}
        if(step.operation==ChatOperation::KeyDown)held_=step.key;
        if(step.operation==ChatOperation::KeyUp)held_=0;
        due_=now+step.wait;++step_;
    }
};
}
