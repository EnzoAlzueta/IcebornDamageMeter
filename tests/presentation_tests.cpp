#include "../src/panel.hpp"
#include "../src/share_chat.hpp"
#include "../src/chat_input.hpp"
#include "../src/game_weapon_icons.hpp"
#include <sstream>
#include <cstdio>
#include <cstdlib>
using namespace meter;
static int failures=0;
#define CHECK(test) do {if(!(test)){std::printf("FAIL %d: %s\n",__LINE__,#test);++failures;}}while(0)
int main() {
    setLanguage(Language::Spanish);
    CHECK(questName(502)==L"Trono ardiente sobre el bosque");
    CHECK(questObjective(502).find(L"Rathalos")!=std::wstring::npos);
    CHECK(questName(999999)==L"Misión #999999");
    CHECK(questName(408)==L"Legiana, el cazador elegante");
    CHECK(questName(251)==L"El gran glotón");
    CHECK(questName(30000)==L"Investigación");
    CHECK(questName(30001)==L"Misión #30001");
    Meter model;Sample sample;sample.valid=true;sample.countersValid=true;sample.questId=502;sample.state=2;
    sample.players[0]={1,L"Enzo",24500,true,true,0};sample.players[1]={2,L"Amigo",45500,false,true,3};
    model.update(sample,1000);
    CHECK(canShareDamage(model,false));CHECK(!canShareDamage(model,true));
    Sample missing;model.update(missing,1100);
    CHECK(!model.dataAvailable()&&canShareDamage(model,false));
    const auto key=chatKeyInput(VK_INSERT,false),up=chatKeyInput(VK_INSERT,true);
    CHECK(key.ki.wVk==0&&key.ki.wScan!=0&&(key.ki.dwFlags&KEYEVENTF_SCANCODE));
    CHECK(key.ki.dwFlags&KEYEVENTF_EXTENDEDKEY);CHECK(up.ki.dwFlags&KEYEVENTF_KEYUP);
    std::array<bool,14> used{};
    // Native IDs differ from the original texture's order for ranged weapons.
    CHECK(WeaponAtlasColumns[11]==13); // Bow: curved bow at atlas column 13
    CHECK(WeaponAtlasColumns[12]==12); // Heavy bowgun: shield-shaped cell 12
    CHECK(WeaponAtlasColumns[13]==11); // Light bowgun: gun silhouette in cell 11
    for(const auto column:WeaponAtlasColumns){CHECK(column<14&&!used[column]);if(column<14)used[column]=true;}
    CHECK(GameWeaponIconsPng.size()>1000&&GameWeaponIconsPng[0]==137);
    {
        Meter field;Visibility visible;
        field.update(sample,100);visible.update(sample);
        auto expedition=sample;expedition.questId=0;expedition.state=0;
        expedition.expedition=true;expedition.stage=101;
        // Stale counters deliberately retained in the input must never leak.
        field.update(expedition,200);visible.update(expedition);
        CHECK(field.expedition()&&field.total()==0&&!canShareDamage(field,false));
        CHECK(field.ranked()[0].id==0&&visible.visible());
        Panel fieldPanel;const auto unavailable=fieldPanel.build(field,200);
        CHECK(unavailable.rowCount==0&&unavailable.title==tr(L"EXPEDITION · NO DATA"));
        field.update(Sample{},300);CHECK(field.expedition()&&field.total()==0);
        auto town=expedition;town.expedition=false;town.stage=301;
        field.update(town,400);visible.update(town);CHECK(!field.expedition()&&!visible.visible());
        for(const int stage:{101,102,103,104,105,108,109})CHECK(isExpeditionField(0,0,stage));
        for(const int stage:{0,201,202,203,301,305,504})CHECK(!isExpeditionField(0,0,stage));
        // Actual catalog IDs: Arena Quest 01, Special Arena and Greatest Jagras event.
        for(const int id:{3101,331,61606}) {
            auto mission=sample;mission.questId=id;mission.stage=id==61606?101:202;
            CHECK(questInfo(id)&&!isExpeditionField(id,2,mission.stage));
            field.update(mission,1000);visible.update(mission);
            CHECK(!field.expedition()&&field.phase()==Phase::Live&&field.total()==70000&&visible.visible());
            mission.state=3;field.update(mission,2000);field.update(mission,5000);
            CHECK(field.phase()==Phase::Summary&&canShareDamage(field,false));
        }
    }
    {
        Meter completed;completed.update(sample,100);
        auto end=sample;end.state=3;completed.update(end,1000);completed.update(end,4000);
        CHECK(completed.phase()==Phase::Summary&&canShareDamage(completed,false));
        completed.update(Sample{},4100);
        CHECK(!completed.dataAvailable()&&canShareDamage(completed,false));
        auto next=sample;next.questId=251;next.players[0].damage=0;next.players[1].damage=0;
        completed.update(next,4200);CHECK(!canShareDamage(completed,false));
    }
    const auto record=captureHunt(model,2000,1794435452000ULL);
    Panel panel;panel.history.add(record);
    CHECK(panel.config.showBadge&&panel.config.autoShow);
    panel.change(Action::Decrease,14);panel.change(Action::Decrease,15);
    CHECK(!panel.config.showBadge&&!panel.config.autoShow);
    panel.change(Action::Settings);const auto settingsView=panel.build(model,2000);
    CHECK(settingsView.rowCount==16);
    CHECK(settingsView.rows[14].name==tr(L"Show DM button")&&settingsView.rows[14].value==tr(L"No"));
    CHECK(settingsView.rows[15].name==tr(L"Show on quest start")&&settingsView.rows[15].value==tr(L"No"));
    panel.change(Action::Current);
    auto view=panel.build(model,2000);
    CHECK(view.title.find(questName(502))!=std::wstring::npos);
    panel.change(Action::History);view=panel.build(model,2000);
    CHECK(view.historyList&&view.rows[0].name==questName(502)&&!view.rows[0].subtitle.empty());
    panel.change(Action::SelectRecord,0);view=panel.build(model,2000);
    CHECK(view.title==questName(502));CHECK(view.comparison.find(L"Rathalos")!=std::wstring::npos);
    // Existing IDMHST03 histories retain their binary format and gain names.
    std::stringstream oldHistory(std::ios::in|std::ios::out|std::ios::binary);
    CHECK(history_codec::save(oldHistory,panel.history));History loaded;
    CHECK(oldHistory.str().substr(0,8)=="IDMHST03");CHECK(history_codec::load(oldHistory,loaded));
    CHECK(questName(loaded.get(0)->questId)==questName(502));
    for(int i=0;i<100;++i)panel.change(Action::Decrease,0);
    CHECK(panel.config.scale==40);
    for(int i=0;i<100;++i)panel.change(Action::Decrease,2);
    CHECK(panel.config.width==260);
    for(int width:{260,280,340,360,400,460,640})for(auto page:{PanelPage::Current,PanelPage::Details,PanelPage::History,PanelPage::Record,PanelPage::Settings,PanelPage::Menu}) {
        panel.config.width=width;panel.page=page;view=panel.build(model,2000);
        const auto g=rowGeometry(view,static_cast<float>(width));
        CHECK(g.nameWidth>0&&g.valueWidth>0&&g.subtitleWidth>0);
        CHECK(g.nameX+g.nameWidth<=width-10&&g.valueX+g.valueWidth<=width-10);
        CHECK(view.rowHeight>=g.valueY+20);
        if(view.historyList)CHECK(view.rowHeight>=g.subtitleY+20);
        for(size_t i=0;i<view.buttonCount;++i) {
            const auto r=view.buttons[i].rect;
            CHECK(r.x>=0&&r.y>=0&&r.x+r.width<=width&&r.y+r.height<=view.height);
        }
    }
    auto lines=damageSummaryLines(record);
    CHECK(lines.size()>=2&&lines[0].find(L"70000")!=std::wstring::npos);
    std::wstring all;for(const auto& line:lines){CHECK(line.size()<=80);all+=line;}
    CHECK(all.find(L"Enzo: 24500 (35.0%)")!=std::wstring::npos);
    CHECK(all.find(L"Amigo: 45500 (65.0%)")!=std::wstring::npos);
    CHECK(damageChatLines(HuntRecord{}).empty());
    HuntRecord unusual=record;unusual.players[0].name=L"Nombre\n\r\tcon control";
    for(const auto& line:damageChatLines(unusual))CHECK(line.find_first_of(L"\n\r\t")==std::wstring::npos);
    unusual.players[0].name=L"/command";
    for(const auto& line:damageChatLines(unusual))CHECK(line.rfind(L"[Daño] ",0)==0);
    for(size_t i=0;i<unusual.players.size();++i)unusual.players[i]={i+1,L"Nombre largo de cazador",100000000,false,false,0};
    auto crowded=damageSummaryLines(unusual);size_t entries=0;
    for(const auto& line:crowded){CHECK(line.size()<=80);for(const auto c:line)if(c==L'%')++entries;}
    CHECK(entries==16);
    const auto compact=damageChatLines(record);
    CHECK(compact.size()==1&&compact[0].find(L"24500")!=std::wstring::npos&&compact[0].find(L"45500")!=std::wstring::npos);
    // Four maximum counters must fit even with long translated prefixes/names.
    for(unsigned locale=0;locale<LanguageCount;++locale) {
        setLanguage(static_cast<Language>(locale));
        HuntRecord four=record;four.players={};
        for(const auto& name:{L"Long hunter name",L"日本語のプレイヤー",L"РусскийИгрок",L"لاعب بالعربية"}) {
            for(size_t i=0;i<4;++i)four.players[i]={i+1,name,100000000,false,true,0};
            const auto shortLines=damageChatLines(four);
            size_t players=0;
            for(const auto& line:shortLines){CHECK(line.size()<=80&&chatUtf8Bytes(line)<=127);players+=std::count(line.begin(),line.end(),L':');}
            CHECK(players==4);
            std::wstring joined;for(const auto& line:shortLines)joined+=line;
            CHECK(joined.find(name)!=std::wstring::npos);
        }
    }
    setLanguage(Language::Spanish);
    {
        auto named=record;named.players[1].name=L"Pharis";
        const auto namedLines=damageChatLines(named);
        CHECK(namedLines.size()==1&&namedLines[0].find(L"Pharis: 24500")!=std::wstring::npos);
        auto investigation=record;investigation.questId=30000;
        History investigations;investigations.add(investigation);
        CHECK(investigations.comparable(investigation)==nullptr);
        Meter city;city.update(sample,100);auto end=sample;end.state=3;
        city.update(end,1000);city.update(end,4000);
        Sample town;town.valid=true;town.state=0;city.update(town,5000);
        CHECK(city.phase()==Phase::Summary&&canShareDamage(city,false)&&city.total()==70000);
    }
    std::wstring surrogate{wchar_t(0xd83d),wchar_t(0xde00)};
    CHECK(chatText(surrogate,1).empty());
    for(int open:{0x2d,0x0d}) {
        ChatPlayback playback;std::vector<ChatStep> emitted;
        const auto emit=[&](const ChatStep& step){emitted.push_back(step);return true;};
        CHECK(playback.start(lines,100,open));CHECK(!playback.start(lines,101,open));
        playback.tick(300,true,true,false,emit);CHECK(emitted.empty());
        for(unsigned now=350;now<60000;now+=50)playback.tick(now,true,false,false,emit);
        CHECK(playback.result()==ChatResult::Completed);
        std::vector<std::wstring> delivered;int held=0;
        for(const auto& step:emitted) {
            if(step.operation==ChatOperation::KeyDown){CHECK(!held);held=step.key;}
            if(step.operation==ChatOperation::KeyUp){CHECK(held==step.key);held=0;}
            if(step.operation==ChatOperation::Text){if(delivered.empty())delivered.emplace_back();delivered.back()+=step.text;CHECK(step.text.size()<=2);}
            if(step.operation==ChatOperation::KeyDown&&step.key==0x0d&&!delivered.empty()&&!delivered.back().empty())delivered.emplace_back();
        }
        if(!delivered.empty()&&delivered.back().empty())delivered.pop_back();
        CHECK(delivered==lines&&!held);
        CHECK(playback.start(lines,61000,open));emitted.clear();
        playback.tick(61200,true,false,false,emit);playback.tick(61201,false,false,false,emit);
        CHECK(playback.result()==ChatResult::Cancelled&&emitted.back().operation==ChatOperation::KeyUp);
    }
    ChatPlayback failing;CHECK(failing.start(lines,1,0x2d));
    failing.tick(201,true,false,false,[](const ChatStep&){return false;});
    CHECK(failing.result()==ChatResult::Failed);
    std::puts(failures?"Presentation/chat tests FAILED":"Quest labels, history compatibility, narrow layouts and manual chat playback passed");
    return failures?1:0;
}
