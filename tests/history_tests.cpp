#include "../src/panel.hpp"
#include <sstream>
#include <cstdio>
#include <cmath>
using namespace meter;
static int failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(0)
static Sample sample(int state,std::uint32_t damage) {
    Sample s;s.valid=s.countersValid=true;s.questId=42;s.state=state;
    s.players[0]={1,L"Enzo",damage,true,true,0};return s;
}
static HuntRecord record(std::uint64_t stamp=1000) {
    HuntRecord r;r.timestamp=stamp;r.questId=42;r.durationMs=60000;r.partySize=1;r.outcome=3;
    r.players[0]={1,L"Enzo",3000,true,true,0};r.players[0].participationMs=60000;return r;
}
int main(){
    setLanguage(Language::Spanish);
    {
        Meter m;m.update(sample(1,0),0);m.update(sample(2,0),100);
        m.update(sample(2,100),1100);CHECK(m.ranked()[0].participationMs==1000);CHECK(m.ranked()[0].dps()==100);
        m.update(sample(2,100),2100);CHECK(m.ranked()[0].dps()==50); // downtime counts
        m.update(sample(3,150),3100);m.update(sample(3,175),4100);m.update(sample(0,0),6000);
        const auto r=captureHunt(m,6000,1000);CHECK(r.durationMs==3000);CHECK(r.players[0].participationMs==3000);
        CHECK(std::abs(r.players[0].dps()-175.0/3)<0.001);
    }
    {
        Meter m;m.update(sample(2,1000),100);m.update(sample(2,1100),1100);
        CHECK(m.ranked()[0].dps()==100);CHECK(m.ranked()[0].baselineDamage==1000);
        CHECK(m.partial()); // never divide inherited damage by our observation time
        auto s=sample(2,1200);s.players[1]={2,L"Late join",500,false,true,3};m.update(s,2100);
        s.players[1].damage=650;m.update(s,3100);CHECK(m.ranked()[1].dps()==150);
        s.players[1]={};m.update(s,4100);const auto elapsed=m.ranked()[1].participationMs;
        m.update(s,6100);CHECK(m.ranked()[1].participationMs==elapsed);
    }
    {
        History history;
        for(unsigned i=0;i<12;++i)history.add(record(1000+i));
        CHECK(history.size()==10 && history.get(0)->timestamp==1011 && history.get(9)->timestamp==1002);
        std::stringstream stream(std::ios::in|std::ios::out|std::ios::binary);
        CHECK(history_codec::save(stream,history));History loaded;CHECK(history_codec::load(stream,loaded));
        CHECK(loaded.size()==10 && loaded.get(0)->players[0].name==L"Enzo");
        auto text=stream.str();text.resize(text.size()-1);std::istringstream truncated(text);
        CHECK(!history_codec::load(truncated,loaded));CHECK(loaded.size()==10);
        text=stream.str();text[8]=100;std::istringstream excessive(text);CHECK(!history_codec::load(excessive,loaded));
        text=stream.str();text[0]='X';std::istringstream wrongVersion(text);CHECK(!history_codec::load(wrongVersion,loaded));
        auto current=record(2000);CHECK(history.comparable(current)!=nullptr);
        current.questId=43;CHECK(history.comparable(current)==nullptr);current.questId=42;
        current.players[0].weapon=3;CHECK(history.comparable(current)==nullptr);current.players[0].weapon=0;
        current.players[0].changedWeapon=true;CHECK(history.comparable(current)==nullptr);current.players[0].changedWeapon=false;
        current.players[0].baselineDamage=10;CHECK(history.comparable(current)==nullptr);current.players[0].baselineDamage=0;
        current.partySize=2;CHECK(history.comparable(current)==nullptr);current.partySize=1;
        current.partial=true;CHECK(history.comparable(current)==nullptr);current.partial=false;
        current.outcome=5;CHECK(history.comparable(current)==nullptr);current.outcome=3;
        current.timestamp=1002;CHECK(history.comparable(current)==nullptr); // no comparing an old record to a newer one
    }
    {
        Panel panel;Meter m;auto view=panel.build(m,0);CHECK(view.rowCount==0);
        panel.change(Action::Settings);view=panel.build(m,0);CHECK(view.settings && view.rowCount==16);
        const int old=panel.config.scale;CHECK(panel.change(Action::Increase,0));CHECK(panel.config.scale==old+5);
        panel.change(Action::Increase,5);CHECK(!panel.config.dps);
        for(int i=0;i<100;++i)panel.change(Action::Increase,0);CHECK(panel.config.scale==175);
        for(int i=0;i<24;++i){panel.change(Action::Increase,9);CHECK(panel.config.showKey!=panel.config.settingsKey);CHECK(panel.config.showKey!=panel.config.moveKey);}
        panel.history.add(record());panel.change(Action::History);view=panel.build(m,0);CHECK(view.rowCount==1);
        panel.change(Action::SelectRecord,0);view=panel.build(m,0);CHECK(view.rowCount==1 && view.rows[0].name==L"Enzo");
        panel.change(Action::Current);view=panel.build(m,0);CHECK(view.rowCount==0);
        CHECK(view.compact && view.footer.empty() && view.columns.empty() && view.buttonCount==1);
        m.update(sample(1,0),0);m.update(sample(2,0),100);
        view=panel.build(m,100);CHECK(view.rowCount==1 && view.height<60);
        panel.change(Action::Menu);view=panel.build(m,100);CHECK(view.menu && view.rowCount==8);
        bool details=false,history=false,settings=false;
        for(size_t i=0;i<view.buttonCount;++i) {
            const auto action=view.buttons[i].action;
            if(action==Action::Details)details=true;
            if(action==Action::History)history=true;
            if(action==Action::Settings)settings=true;
        }
        CHECK(details&&history&&settings);
        panel.change(Action::Details);view=panel.build(m,100);CHECK(!view.compact && !view.footer.empty());
        panel.change(Action::Current);view=panel.build(m,100);CHECK(view.compact && view.comparison.empty());
        PanelConfig bad;bad.showKey=bad.settingsKey;bad.scale=999;bad.normalize();CHECK(bad.showKey!=bad.settingsKey && bad.scale==175);
    }
    std::puts(failures?"History/DPS/panel tests FAILED":"History, DPS, comparison and panel tests passed");return failures?1:0;
}
