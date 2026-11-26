#include "../src/panel.hpp"
#include "../src/damage_summary.hpp"
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
    CHECK(canCopyDamage(model,false));CHECK(!canCopyDamage(model,true));
    Sample missing;model.update(missing,1100);
    CHECK(!model.dataAvailable()&&canCopyDamage(model,false));
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
        CHECK(field.expedition()&&field.total()==0&&!canCopyDamage(field,false));
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
            CHECK(field.phase()==Phase::Summary&&canCopyDamage(field,false));
        }
    }
    {
        Meter field;
        Visibility visible;
        unsigned archived = 0;
        const auto poll = [&](const Sample& observation, std::uint64_t now) {
            updateHunt(field, observation, now, [&](bool) { ++archived; });
            visible.update(observation);
        };
        poll(sample, 100);
        Sample guiding;
        guiding.valid = guiding.expedition = true;
        guiding.stage = GuidingLandsStage;
        poll(guiding, 200);
        CHECK(archived==1&&visible.visible()&&field.guidingLands()&&field.total()==0&&!canCopyDamage(field,false));
        guiding.countersValid = true;
        guiding.players[0] = {1,L"Enzo",0,true,true,0};
        poll(guiding, 300);
        const auto visit = field.serial();
        guiding.players[0].damage = 150;
        poll(guiding, 1300);
        poll(guiding, 1300);
        CHECK(field.serial()==visit&&field.total()==150&&field.ranked()[0].dps()==150.0);
        Panel fieldPanel;
        const auto personal = fieldPanel.build(field,1300);
        CHECK(personal.rowCount==1&&personal.title==L"Tierras Destino · Daño propio");
        CHECK(personal.rows[0].value.find(L'%')==std::wstring::npos);
        CHECK(canCopyDamage(field,false)&&!canCopyDamage(field,true));
        const auto copied = damageSummaryLines(captureHunt(field,1300,0));
        CHECK(copied.size()==2&&copied[1].find(L'%')==std::wstring::npos);
        poll(Sample{},1400);
        CHECK(field.guidingLands()&&field.total()==150&&!field.dataAvailable());
        auto town = guiding;
        town.stage = 305;
        town.expedition = false;
        town.countersValid = false;
        poll(town,1500);
        CHECK(!field.guidingLands()&&!visible.visible()&&field.total()==0&&!canCopyDamage(field,false));
        guiding.players[0].damage = 0;
        poll(guiding,1600);
        CHECK(field.serial()!=visit&&field.total()==0&&visible.visible());
        auto questInArea = sample;
        questInArea.questId = 51606;
        questInArea.stage = GuidingLandsStage;
        questInArea.expedition = isExpeditionField(questInArea.questId, questInArea.state, questInArea.stage);
        poll(questInArea,1700);
        CHECK(archived==1&&!field.expedition()&&field.total()==70000&&field.questId()==51606);
    }
    {
        Meter completed;completed.update(sample,100);
        auto end=sample;end.state=3;completed.update(end,1000);completed.update(end,4000);
        CHECK(completed.phase()==Phase::Summary&&canCopyDamage(completed,false));
        completed.update(Sample{},4100);
        CHECK(!completed.dataAvailable()&&canCopyDamage(completed,false));
        auto next=sample;next.questId=251;next.players[0].damage=0;next.players[1].damage=0;
        completed.update(next,4200);CHECK(!canCopyDamage(completed,false));
    }
    const auto record=captureHunt(model,2000,1794435452000ULL);
    Panel panel;panel.history.add(record);
    CHECK(panel.config.showBadge&&panel.config.autoShow);
    panel.change(Action::Decrease,13);panel.change(Action::Decrease,14);
    CHECK(!panel.config.showBadge&&!panel.config.autoShow);
    panel.change(Action::Settings);const auto settingsView=panel.build(model,2000);
    CHECK(settingsView.rowCount==15);
    CHECK(settingsView.rows[13].name==tr(L"Show DM button")&&settingsView.rows[13].value==tr(L"No"));
    CHECK(settingsView.rows[14].name==tr(L"Show on quest start")&&settingsView.rows[14].value==tr(L"No"));
    panel.change(Action::Menu);CHECK(panel.build(model,2000).rowCount==7);
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
    float liveHeight=0,settingsHeight=0;
    for(int width:{260,280,340,359,360,400,460,640})for(auto page:{PanelPage::Current,PanelPage::Details,PanelPage::History,PanelPage::Record,PanelPage::Settings,PanelPage::Menu}) {
        panel.config.width=width;panel.page=page;view=panel.build(model,2000);
        const auto g=rowGeometry(view,static_cast<float>(width));
        if(page==PanelPage::Current) {
            CHECK(!view.narrow&&view.rowHeight==25&&g.valueY==3);
            if(!liveHeight)liveHeight=view.height;
            CHECK(view.height==liveHeight);
            CHECK(g.nameX+g.nameWidth+8<=g.valueX);
        }
        if(page==PanelPage::Settings) {
            if(!settingsHeight)settingsHeight=view.height;
            CHECK(view.height==settingsHeight&&view.rowHeight==25&&g.valueY==g.nameY);
            for(size_t i=0;i<view.buttonCount;++i) {
                const auto& button=view.buttons[i];
                if(button.action!=Action::Increase&&button.action!=Action::Decrease)continue;
                CHECK(button.rect.y==view.bodyTop+button.data*view.rowHeight+1);
                CHECK(g.valueX+g.valueWidth<=button.rect.x);
            }
        }
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
    HuntRecord unusual=record;unusual.players[0].name=L"Nombre\n\r\tcon control";
    for(const auto& line:damageSummaryLines(unusual))CHECK(line.find_first_of(L"\n\r\t")==std::wstring::npos);
    for(size_t i=0;i<unusual.players.size();++i)unusual.players[i]={i+1,L"Nombre largo de cazador",100000000,false,false,0};
    auto crowded=damageSummaryLines(unusual);size_t entries=0;
    for(const auto& line:crowded){CHECK(line.size()<=80);for(const auto c:line)if(c==L'%')++entries;}
    CHECK(entries==16);
    {
        auto investigation=record;investigation.questId=30000;
        History investigations;investigations.add(investigation);
        CHECK(investigations.comparable(investigation)==nullptr);
        Meter city;city.update(sample,100);auto end=sample;end.state=3;
        city.update(end,1000);city.update(end,4000);
        Sample town;town.valid=true;town.state=0;city.update(town,5000);
        CHECK(city.phase()==Phase::Summary&&canCopyDamage(city,false)&&city.total()==70000);
    }
    std::wstring surrogate{wchar_t(0xd83d),wchar_t(0xde00)};
    CHECK(summaryText(surrogate,1).empty());
    std::puts(failures?"Presentation/clipboard tests FAILED":"Quest labels, history compatibility, narrow layouts and clipboard summaries passed");
    return failures?1:0;
}
