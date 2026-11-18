#pragma once
#include "quest_catalog.hpp"
#include "language.hpp"
#include <algorithm>
#include <iterator>
#include <string>
namespace meter {
static_assert(QuestLocaleCount==LanguageCount,"Quest catalog and runtime locale order must match");
inline const QuestInfo* questInfo(int id) {
    const auto end=std::end(QuestCatalog);
    const auto found=std::lower_bound(std::begin(QuestCatalog),end,id,
        [](const QuestInfo& quest,int value){return quest.id<value;});
    return found!=end&&found->id==id?found:nullptr;
}
inline std::wstring questName(int id) {
    if(id==30000)return tr(L"Investigation");
    if(const auto info=questInfo(id))return info->names[static_cast<size_t>(currentLanguage())];
    return id>0?tr(L"Quest #")+std::to_wstring(id):tr(L"No quest");
}
inline std::wstring questObjective(int id) {
    if(const auto info=questInfo(id))return info->objectives[static_cast<size_t>(currentLanguage())];
    return {};
}
}
