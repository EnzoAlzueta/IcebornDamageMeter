#pragma once
#include "share_chat.hpp"
#include <string>
namespace meter {
enum class NativeChatState { Unavailable,Busy,Ready };
// A consumed game request is not proof of network delivery. Never retry a
// submitted message: an uncertain result must not create duplicate chat spam.
class NativeChatPlayback {
    std::vector<std::string> lines_;
    size_t next_=0;
    bool pending_=false;
    std::uint64_t due_=0,deadline_=0,cooldown_=0;
    ChatResult result_=ChatResult::Idle;
public:
    ChatResult result() const{return result_;}
    bool busy() const{return result_==ChatResult::Sending;}
    bool start(std::vector<std::string> lines,std::uint64_t now) {
        if(busy()||now<cooldown_||lines.empty()||lines.size()>16)return false;
        for(const auto& line:lines)if(line.empty()||line.size()>127||line.find('\0')!=std::string::npos)return false;
        lines_=std::move(lines);next_=0;pending_=false;due_=now+200;
        deadline_=now+5000;cooldown_=now+15000;result_=ChatResult::Sending;return true;
    }
    template<class Probe,class Submit> void tick(std::uint64_t now,bool focused,bool mouseDown,bool escape,Probe probe,Submit submit) {
        if(!busy())return;
        const auto stop=[&](ChatResult result){result_=result;lines_.clear();pending_=false;};
        if(!focused||escape){stop(ChatResult::Cancelled);return;}
        if(now>deadline_){stop(ChatResult::Failed);return;}
        if(now<due_||(!next_&&!pending_&&mouseDown))return;
        const auto state=probe();
        if(state==NativeChatState::Unavailable){stop(ChatResult::Failed);return;}
        if(state==NativeChatState::Busy)return;
        if(pending_) {
            pending_=false;++next_;
            if(next_==lines_.size()){stop(ChatResult::Completed);return;}
            due_=now+1500;deadline_=due_+5000;return;
        }
        if(!submit(lines_[next_])){stop(ChatResult::Failed);return;}
        pending_=true;deadline_=now+5000;
    }
};
}
