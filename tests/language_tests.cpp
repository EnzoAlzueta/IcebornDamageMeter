#include "../src/panel.hpp"
#include "../src/damage_summary.hpp"
#include <cstdio>
using namespace meter;
static int failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(0)
int main() {
    CHECK(parseLanguage("brazilian")==Language::Portuguese);
    CHECK(parseLanguage("ES_ar")==Language::SpanishLatinAmerica);
    CHECK(parseLanguage("latam")==Language::SpanishLatinAmerica);
    CHECK(parseLanguage("es-419")==Language::SpanishLatinAmerica);
    CHECK(parseLanguage("spanish")==Language::Spanish);
    CHECK(parseLanguage("koreana")==Language::Korean);
    CHECK(parseLanguage("schinese")==Language::ChineseSimplified);
    CHECK(parseLanguage("tchinese")==Language::ChineseTraditional);
    CHECK(!parseLanguage("auto")&&!parseLanguage("unsupported"));
    CHECK(selectAutomaticLanguage("german","spanish","en-US")==Language::German);
    CHECK(selectAutomaticLanguage("","spanish","en-US")==Language::Spanish);
    CHECK(selectAutomaticLanguage("","","pt_BR")==Language::Portuguese);
    CHECK(selectAutomaticLanguage("invalid","invalid","invalid")==Language::English);
    CHECK(manifestLanguage("\"UserConfig\" { \"language\" \"japanese\" }")=="japanese");
    CHECK(manifestLanguage("\"language\"").empty());
    Meter model;Sample sample;sample.valid=sample.countersValid=true;sample.state=2;sample.questId=103;
    sample.players[0]={1,L"ActualPlayer",1000,true,true,0};model.update(sample,0);
    for(size_t i=0;i<LanguageCount;++i) {
        setLanguage(static_cast<Language>(i));
        const auto epoch=languageRevision.load();setLanguage(static_cast<Language>(i));CHECK(languageRevision.load()==epoch);
        for(const auto& row:UiTranslations)CHECK(tr(row.source)&&*tr(row.source));
        CHECK(!questName(103).empty()&&!questObjective(103).empty());
        CHECK(questName(999999).find(L"999999")!=std::wstring::npos);
        Panel panel;auto view=panel.build(model,1000);CHECK(view.rows[0].name==L"ActualPlayer");
        CHECK(view.title.find(questName(103))!=std::wstring::npos);
        panel.change(Action::Settings);view=panel.build(model,1000);CHECK(view.rows[0].name==tr(L"Scale"));
        panel.change(Action::Menu);view=panel.build(model,1000);CHECK(view.rows[0].name==tr(L"Current quest details"));
        const auto lines=damageSummaryLines(captureHunt(model,1000,0));
        CHECK(lines.size()==2);
        for(const auto& line:lines){CHECK(line.size()<=80);CHECK(line.rfind(tr(L"[Damage] "),0)==0);}
        CHECK(lines.size()>1&&lines[1].find(L"ActualPlayer")!=std::wstring::npos);
        CHECK(weaponStyle(0).name==std::wstring(tr(L"Great Sword")));
    }
    setLanguage(Language::English);CHECK(std::wstring(tr(L"Settings"))==L"Settings");
    CHECK(questName(103)==L"The Great Jagras Hunt");
    setLanguage(Language::Spanish);CHECK(questName(103)==L"La cacería del Gran Jagras");
    setLanguage(Language::ChineseSimplified);
    CHECK(questName(103)==L"狩猎大贼龙吧");
    CHECK(questName(1591)==L"翔风之龙"&&questObjective(1591)==L"讨伐钢龙");
    CHECK(questName(30000)==tr(L"Investigation")&&questObjective(30000).empty());
    setLanguage(Language::ChineseTraditional);CHECK(questName(1591)==L"翔風之龍");
    setLanguage(Language::SpanishLatinAmerica);
    CHECK(questName(1225)==L"Si nos ayudas...");
    CHECK(std::wstring(tr(L"Settings"))==L"Ajustes");
    setLanguage(static_cast<Language>(99));CHECK(currentLanguage()==Language::English);
    std::puts(failures?"Language tests FAILED":"13 UI languages, 14 game-text locales: quests, clipboard, priority and cache revision passed");
    return failures?1:0;
}
