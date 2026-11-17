#pragma once
#include "history.hpp"
#include "quests.hpp"
#include <vector>
#include <cstdio>

namespace meter {
inline bool canCopyDamage(const Meter& model,bool preview) {
    return !preview&&model.questId()>0&&model.total()>0;
}
inline std::wstring summaryText(const std::wstring& text,size_t limit) {
    if(!limit)return {};
    std::wstring clean;
    for(const auto c:text) {
        if(c>=32&&c!=127&&c!=L'\u2028'&&c!=L'\u2029')clean+=c;
        if(clean.size()==limit)break;
    }
    if(!clean.empty()&&clean.back()>=0xd800&&clean.back()<=0xdbff)clean.pop_back();
    return clean;
}
// Clipboard report; this helper never submits text to the game.
inline std::vector<std::wstring> damageSummaryLines(const HuntRecord& record) {
    if(record.questId<=0||!record.total())return {};
    std::vector<std::wstring> lines;
    lines.push_back(tr(L"[Damage] ")+summaryText(questName(record.questId),36)+tr(L" | Total: ")+std::to_wstring(record.total()));
    const std::wstring prefix=tr(L"[Damage] ");
    std::wstring line=prefix;
    for(const auto& player:record.players)if(player.id) {
        wchar_t percent[24]{};
        swprintf_s(percent,L" (%.1f%%)",100.0*player.damage/record.total());
        const auto item=summaryText(player.name,12)+L": "+std::to_wstring(player.damage)+percent;
        if(line.size()>prefix.size()&&line.size()+3+item.size()>80) {lines.push_back(line);line=prefix;}
        if(line.size()>prefix.size())line+=L" | ";
        line+=item;
    }
    if(line.size()>prefix.size())lines.push_back(line);
    return lines;
}
}
