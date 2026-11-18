#pragma once
#include "translations_data.hpp"
#include <algorithm>
#include <atomic>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
namespace meter {
// Latin American Spanish reuses Spanish UI strings, with its own game text.
enum class Language:unsigned {English,Spanish,Portuguese,French,German,Italian,Japanese,Korean,ChineseSimplified,ChineseTraditional,Russian,Polish,Arabic,SpanishLatinAmerica,Count};
inline constexpr size_t LanguageCount=static_cast<size_t>(Language::Count);
inline std::atomic<Language> activeLanguage{Language::English};
inline std::atomic<unsigned> languageRevision{0};
inline Language currentLanguage(){return activeLanguage.load(std::memory_order_relaxed);}
inline void setLanguage(Language language) {
    if(language>=Language::Count)language=Language::English;
    if(activeLanguage.exchange(language)!=language)++languageRevision;
}
inline const wchar_t* tr(const wchar_t* source) {
    const auto language=currentLanguage();if(language==Language::English)return source;
    const std::wstring_view key(source);const auto end=std::end(UiTranslations);
    const auto found=std::lower_bound(std::begin(UiTranslations),end,key,
        [](const UiTranslation& row,std::wstring_view value){return std::wstring_view(row.source)<value;});
    if(found==end||std::wstring_view(found->source)!=key)return source;
    const auto ordinal=static_cast<unsigned>(language==Language::SpanishLatinAmerica?Language::Spanish:language);
    return found->values[ordinal-1];
}
inline const wchar_t* languageTag() {
    constexpr const wchar_t* tags[]={L"en-US",L"es-ES",L"pt-BR",L"fr-FR",L"de-DE",L"it-IT",L"ja-JP",L"ko-KR",L"zh-CN",L"zh-TW",L"ru-RU",L"pl-PL",L"ar-SA",L"es-419"};
    return tags[static_cast<unsigned>(currentLanguage())];
}
inline const char* languageCode() {
    constexpr const char* codes[]={"en","es","pt-BR","fr","de","it","ja","ko","zh-CN","zh-TW","ru","pl","ar","es-419"};
    return codes[static_cast<unsigned>(currentLanguage())];
}
inline std::optional<Language> parseLanguage(std::string_view input) {
    std::string value;for(const char c:input)if(c!=' '&&c!='\t')value+=c=='_'?'-':c>='A'&&c<='Z'?char(c-'A'+'a'):c;
    if(value=="english"||value=="en"||value.rfind("en-",0)==0)return Language::English;
    if(value=="latam"||(value.rfind("es-",0)==0&&value!="es-es"))return Language::SpanishLatinAmerica;
    if(value=="spanish"||value=="es"||value=="es-es")return Language::Spanish;
    if(value=="brazilian"||value=="portuguese"||value=="pt"||value.rfind("pt-",0)==0)return Language::Portuguese;
    if(value=="french"||value=="fr"||value.rfind("fr-",0)==0)return Language::French;
    if(value=="german"||value=="de"||value.rfind("de-",0)==0)return Language::German;
    if(value=="italian"||value=="it"||value.rfind("it-",0)==0)return Language::Italian;
    if(value=="japanese"||value=="ja"||value.rfind("ja-",0)==0)return Language::Japanese;
    if(value=="koreana"||value=="korean"||value=="ko"||value.rfind("ko-",0)==0)return Language::Korean;
    if(value=="schinese"||value=="zh-cn"||value=="zh-sg"||value=="zh-hans"||value=="zh")return Language::ChineseSimplified;
    if(value=="tchinese"||value=="zh-tw"||value=="zh-hk"||value=="zh-hant")return Language::ChineseTraditional;
    if(value=="russian"||value=="ru"||value.rfind("ru-",0)==0)return Language::Russian;
    if(value=="polish"||value=="pl"||value.rfind("pl-",0)==0)return Language::Polish;
    if(value=="arabic"||value=="ar"||value.rfind("ar-",0)==0)return Language::Arabic;
    return std::nullopt;
}
inline Language selectAutomaticLanguage(std::string_view steamGame,std::string_view manifest,std::string_view windows) {
    for(const auto value:{steamGame,manifest,windows})if(const auto parsed=parseLanguage(value))return *parsed;
    return Language::English;
}
inline std::string manifestLanguage(std::string_view text) {
    size_t offset=0;
    while((offset=text.find('"',offset))!=text.npos) {
        const auto end=text.find('"',offset+1);if(end==text.npos)break;
        const auto token=text.substr(offset+1,end-offset-1);offset=end+1;
        if(token!="language")continue;
        const auto start=text.find('"',offset);if(start==text.npos)break;
        const auto finish=text.find('"',start+1);if(finish==text.npos)break;
        return std::string(text.substr(start+1,finish-start-1));
    }
    return {};
}
void initializeLanguage(const std::wstring& folder);
void updateGameLanguage();
}
