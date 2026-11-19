#pragma once
#include "history.hpp"
#include "view_state.hpp"
#include "quests.hpp"
#include <cstdio>
#include <ctime>
namespace meter {
struct PanelConfig {
    int scale=100,opacity=92,width=460,maxRows=4;
    bool damage=true,share=true,dps=true,time=true,comparison=true;
    bool showBadge=true,autoShow=true;
    int showKey=0x77,historyKey=0x76,settingsKey=0x75,moveKey=0x78;
    void normalize() {
        scale=std::clamp(scale,40,175); opacity=std::clamp(opacity,55,100);
        width=std::clamp(width,260,640); maxRows=std::clamp(maxRows,4,16);
        int* keys[]={&showKey,&historyKey,&settingsKey,&moveKey};
        const int defaults[]={0x77,0x76,0x75,0x78};
        for(int i=0;i<4;++i) {
            if(*keys[i]<0x70||*keys[i]>0x7b) *keys[i]=defaults[i];
            for(int j=0;j<i;++j) if(*keys[i]==*keys[j]) {
                *keys[i]=0x70;
                for(int k=0;k<i;++k) if(*keys[i]==*keys[k]) { ++*keys[i]; k=-1; }
            }
        }
    }
};
inline std::wstring keyName(int key) { return L"F"+std::to_wstring(key-0x70+1); }
inline std::wstring clockText(std::uint64_t ms) {
    wchar_t s[32]{}; swprintf_s(s,L"%02llu:%02llu",ms/60000,(ms/1000)%60); return s;
}
inline std::wstring recordDate(std::uint64_t stamp) {
    const auto seconds=static_cast<std::time_t>(stamp/1000); std::tm local{};
    if(localtime_s(&local,&seconds)) return tr(L"No date");
    wchar_t s[40]{}; swprintf_s(s,L"%02d/%02d %02d:%02d",local.tm_mday,local.tm_mon+1,local.tm_hour,local.tm_min); return s;
}
inline const wchar_t* outcomeName(int outcome) {
    if(outcome==3||outcome==4) return tr(L"Completed");
    if(outcome==5) return tr(L"Failed");
    if(outcome==6||outcome==7) return tr(L"Withdrawn");
    return tr(L"Ended");
}
enum class PanelPage { Current, Details, Record, History, Settings, Menu };
enum class Action { Current, Details, Last, History, Settings, Menu, Move, Close, SelectRecord, Older, Newer, Decrease, Increase, CopyDamage };
struct UiButton { Rect rect; std::wstring text; Action action=Action::Current; int data=0; bool selected=false; };
struct UiRow {
    std::wstring name,value;
    int weapon=UnknownWeapon;
    float fraction=0;
    bool local=false,colored=true;
    std::wstring subtitle;
};
struct PanelView {
    std::wstring title=tr(L"HUNT LOG"),columns,footer,note,comparison;
    std::array<UiRow,16> rows{};
    size_t rowCount=0;
    std::array<UiButton,48> buttons{};
    size_t buttonCount=0;
    float rowHeight=28,bodyTop=79,height=140;
    bool settings=false,compact=false,menu=false,historyList=false,narrow=false;
    void button(Rect r,std::wstring text,Action action,int data=0,bool selected=false) {
        if(buttonCount<buttons.size()) buttons[buttonCount++]={r,std::move(text),action,data,selected};
    }
};
struct RowGeometry {
    float nameX=44,nameWidth=132,valueX=180,valueWidth=264;
    float nameY=3,valueY=3,subtitleX=44,subtitleY=23,subtitleWidth=132;
};
inline RowGeometry rowGeometry(const PanelView& view,float width) {
    RowGeometry g;
    if(view.settings||view.menu) {
        g.nameX=18;
        g.valueX=view.settings?width-140:width-160;
        g.nameWidth=g.valueX-26;
        g.valueWidth=view.settings?60.0f:144.0f;
    } else {
        g.valueX=view.compact&&width<360?(std::max)(96.0f,width*0.35f):width*0.4f;
        g.nameWidth=g.valueX-52;
        g.valueWidth=width-g.valueX-16;
    }
    if(view.historyList) {
        g.nameWidth=width-60;g.valueY=23;g.subtitleWidth=g.valueX-52;
    }
    if(view.narrow) {
        g.nameWidth=width-g.nameX-16;
        g.valueX=18;g.valueWidth=width-(view.settings?102:36);g.valueY=23;
        if(view.historyList) {g.valueY=43;g.subtitleWidth=width-60;}
    }
    return g;
}
class Panel {
    void changeKey(int& key,int direction) {
        for(int tries=0;tries<12;++tries) {
            key=0x70+(key-0x70+direction+12)%12;
            int matches=0; for(const int used:{config.showKey,config.historyKey,config.settingsKey,config.moveKey}) if(used==key) ++matches;
            if(matches==1) return;
        }
    }
public:
    PanelConfig config;
    History history;
    PanelPage page=PanelPage::Current;
    size_t selected=0;
    bool change(Action action,int data=0) {
        switch(action) {
        case Action::Current:page=PanelPage::Current;break;
        case Action::Details:page=PanelPage::Details;break;
        case Action::Menu:page=page==PanelPage::Menu?PanelPage::Current:PanelPage::Menu;break;
        case Action::Move:page=PanelPage::Current;break;
        case Action::Last:page=PanelPage::Record;selected=0;break;
        case Action::History:page=PanelPage::History;break;
        case Action::Settings:page=PanelPage::Settings;break;
        case Action::SelectRecord:page=PanelPage::Record;selected=static_cast<size_t>(data);break;
        case Action::Older:if(selected+1<history.size())++selected;break;
        case Action::Newer:if(selected)--selected;break;
        case Action::Decrease:case Action::Increase: {
            const int d=action==Action::Increase?1:-1;
            switch(data) {
            case 0:config.scale+=5*d;break;
            case 1:config.opacity+=5*d;break;
            case 2:config.width+=20*d;break;
            case 3:config.damage=!config.damage;break;
            case 4:config.share=!config.share;break;
            case 5:config.dps=!config.dps;break;
            case 6:config.time=!config.time;break;
            case 7:config.comparison=!config.comparison;break;
            case 8:config.maxRows+=d;break;
            case 9:changeKey(config.showKey,d);break;
            case 10:changeKey(config.historyKey,d);break;
            case 11:changeKey(config.settingsKey,d);break;
            case 12:changeKey(config.moveKey,d);break;
            case 13:config.showBadge=!config.showBadge;break;
            case 14:config.autoShow=!config.autoShow;break;
            }
            config.normalize(); return true;
        }
        default:break;
        }
        return false;
    }
    PanelView build(const Meter& model,std::uint64_t now,std::uint64_t activeStamp=0) const {
        PanelView view;
        const float width=static_cast<float>(config.width);
        view.compact=page==PanelPage::Current;
        // The live meter stays one line per hunter at every supported width.
        // Settings also keep each value and its controls beside the label.
        view.narrow=!view.compact&&page!=PanelPage::Settings&&config.width<360;
        view.bodyTop=view.compact?24.0f:55.0f;view.rowHeight=view.compact?25.0f:28.0f;
        if(view.narrow)view.rowHeight=44;
        if(!view.compact)view.button({width-56,4,24,24},L"‹",Action::Current);
        view.button({width-28,view.compact?1.0f:4.0f,24,view.compact?20.0f:24.0f},L"≡",Action::Menu);
        if(page==PanelPage::Menu) {
            view.menu=true;view.title=tr(L"LOG · MENU");view.bodyTop=42;view.rowHeight=view.narrow?44.0f:25.0f;
            const wchar_t* labels[]={tr(L"Current quest details"),tr(L"Last hunt"),tr(L"History"),tr(L"Settings"),tr(L"Move panel"),tr(L"Hide panel"),tr(L"Copy damage summary")};
            const std::wstring hints[]={tr(L"DPS & comparison"),L"",keyName(config.historyKey),keyName(config.settingsKey),
                L"Ctrl+"+keyName(config.moveKey),keyName(config.showKey),tr(L"Clipboard")};
            const Action actions[]={Action::Details,Action::Last,Action::History,Action::Settings,Action::Move,Action::Close,Action::CopyDamage};
            for(int i=0;i<7;++i) {
                auto& row=view.rows[view.rowCount++];row.name=labels[i];row.value=hints[i];row.colored=false;
                view.button({10,view.bodyTop+i*view.rowHeight,width-20,view.rowHeight-2},L"",actions[i]);
            }
            view.footer=L"Ctrl+"+keyName(config.settingsKey)+tr(L" opens/closes this menu");
        } else if(page==PanelPage::Settings) {
            view.settings=true;view.rowHeight=25;view.bodyTop=42;view.title=tr(L"LOG · SETTINGS");
            const wchar_t* labels[]={tr(L"Scale"),tr(L"Opacity"),tr(L"Width"),tr(L"Total damage"),tr(L"Percentage"),tr(L"Observed DPS"),
                tr(L"Participation time"),tr(L"Personal comparison"),tr(L"Visible rows"),tr(L"Show / hide"),tr(L"History"),tr(L"Settings"),tr(L"Move panel"),tr(L"Show DM button"),tr(L"Show on quest start")};
            const bool flags[]={config.damage,config.share,config.dps,config.time,config.comparison};
            for(int i=0;i<15;++i) {
                auto& row=view.rows[view.rowCount++];row.name=labels[i];row.colored=false;
                if(i==0)row.value=std::to_wstring(config.scale)+L"%";
                if(i==1)row.value=std::to_wstring(config.opacity)+L"%";
                if(i==2)row.value=std::to_wstring(config.width);
                if(i>=3&&i<=7)row.value=flags[i-3]?tr(L"Yes"):tr(L"No");
                if(i==8)row.value=std::to_wstring(config.maxRows);
                if(i==9)row.value=keyName(config.showKey);
                if(i==10)row.value=keyName(config.historyKey);
                if(i==11)row.value=keyName(config.settingsKey);
                if(i==12)row.value=L"Ctrl+"+keyName(config.moveKey);
                if(i==13)row.value=config.showBadge?tr(L"Yes"):tr(L"No");
                if(i==14)row.value=config.autoShow?tr(L"Yes"):tr(L"No");
                const float y=view.bodyTop+i*view.rowHeight;
                view.button({width-66,y+1,24,22},L"−",Action::Decrease,i);
                view.button({width-36,y+1,24,22},L"+",Action::Increase,i);
            }
            view.footer=tr(L"Changes are saved automatically");
            view.note=L"Ctrl+"+keyName(config.moveKey)+tr(L" locks the panel");
        } else if(page==PanelPage::History) {
            view.historyList=true;view.rowHeight=view.narrow?64.0f:46.0f;
            view.title=tr(L"LOG · RECENT HUNTS");
            view.columns=tr(L"Duration · Your damage · DPS");
            for(size_t i=0;i<history.size();++i) {
                const auto r=history.get(i);const auto local=r->local();
                auto& row=view.rows[view.rowCount++]; row.colored=false;
                row.name=questName(r->questId);row.subtitle=recordDate(r->timestamp);
                row.weapon=local?local->weapon:UnknownWeapon;
                wchar_t text[96]{};swprintf_s(text,L"%ls · %u · %.1f/s",clockText(r->durationMs).c_str(),local?local->damage:0,local?local->dps():0);
                row.value=text;
                view.button({10,view.bodyTop+static_cast<float>(i)*view.rowHeight,width-20,view.rowHeight-3},L"",Action::SelectRecord,static_cast<int>(i));
            }
            view.footer=tr(L"Select a hunt to view its details");
            view.note=history.size()?tr(L"The ten most recent hunts are kept"):tr(L"No hunts saved yet");
        } else {
            HuntRecord current=captureHunt(model,now,activeStamp);
            const auto record=page==PanelPage::Record?history.get(selected):&current;
            if(record) {
                if(page==PanelPage::Current)view.title=model.phase()==Phase::Live?tr(L"HUNT IN PROGRESS"):tr(L"HUNT LOG");
                for(const auto& p:record->players) if(p.id && view.rowCount<static_cast<size_t>(config.maxRows)) {
                    auto& row=view.rows[view.rowCount++];row.name=p.name;row.weapon=p.weapon;row.local=p.local;
                    if(row.name.size()>16)row.name=row.name.substr(0,15)+L"…";
                    if(!p.present)row.name+=L" ·";
                    row.fraction=record->players[0].damage?static_cast<float>(p.damage)/record->players[0].damage:0;
                    const auto append=[&](const std::wstring& part) { if(!row.value.empty())row.value+=L" · ";row.value+=part; };
                    wchar_t text[64]{};
                    if(config.damage)append(std::to_wstring(p.damage));
                    if(config.share) { swprintf_s(text,L"%.1f%%",record->total()?100.0*p.damage/record->total():0);append(text); }
                    if(config.dps) { if(p.participationMs>=1000) { swprintf_s(text,L"%.1f/s",p.dps());append(text); } else append(L"—/s"); }
                    if(config.time&&!view.compact)append(clockText(p.participationMs));
                }
                const auto label=[&](bool enabled,const wchar_t* text) { if(enabled){if(!view.columns.empty())view.columns+=L" · ";view.columns+=text;} };
                label(config.damage,tr(L"Damage"));label(config.share,L"%");label(config.dps,L"DPS");label(config.time&&!view.compact,tr(L"Participation"));
                view.footer=tr(L"Total: ")+std::to_wstring(record->total())+tr(L" · Time ")+clockText(record->durationMs);
                if(config.comparison&&!view.compact) {
                    const auto previous=history.comparable(*record);const auto local=record->local();
                    if(previous && local && local->participationMs>=1000 && previous->local()->dps()>0) {
                        wchar_t text[128]{};swprintf_s(text,tr(L"Your DPS: %+.1f%% vs last comparable hunt"),100*(local->dps()/previous->local()->dps()-1));view.comparison=text;
                    } else view.comparison=tr(L"Comparison: no matching record");
                }
                if(page==PanelPage::Record) {
                    view.title=tr(L"LOG · ")+recordDate(record->timestamp);
                    view.note=questName(record->questId)+L" · "+outcomeName(record->outcome);
                    view.button({10,view.bodyTop+static_cast<float>(view.rowCount)*view.rowHeight+65,82,22},tr(L"‹ Newer"),Action::Newer);
                    view.button({100,view.bodyTop+static_cast<float>(view.rowCount)*view.rowHeight+65,82,22},tr(L"Older ›"),Action::Older);
                } else view.note=tr(L"DPS since observation · includes pauses and travel");
                if(page==PanelPage::Details) {
                    view.title=questName(record->questId);
                    if(const auto objective=questObjective(record->questId);!objective.empty())view.note=objective;
                }
                if(page==PanelPage::Record) {
                    view.title=questName(record->questId);
                    view.note=recordDate(record->timestamp)+L" · "+outcomeName(record->outcome);
                    if(const auto objective=questObjective(record->questId);!objective.empty())view.comparison=objective;
                }
                size_t participants=0;for(const auto& p:record->players)if(p.id)++participants;
                if(participants>view.rowCount)view.note=std::to_wstring(view.rowCount)+tr(L" of ")+std::to_wstring(participants)+tr(L" players · increase Visible rows");
            } else {view.footer=tr(L"No saved hunts");view.note=tr(L"History starts with this version");}
        }
        if(view.compact) {
            view.title=model.phase()==Phase::Waiting?tr(L"DAMAGE · WAITING FOR QUEST"):tr(L"DAMAGE · ")+questName(model.questId());
            view.columns.clear();view.footer.clear();view.note.clear();view.comparison.clear();
        }
        if(model.expedition()&&(page==PanelPage::Current||page==PanelPage::Details)) {
            view.title=tr(L"EXPEDITION · NO DATA");
            view.note=tr(L"Expedition damage unavailable");
            view.footer.clear();view.comparison.clear();view.columns.clear();
        }
        view.height=view.bodyTop+static_cast<float>(view.rowCount)*view.rowHeight+
            (view.compact?6:view.menu?36:78)+(page==PanelPage::Record?27:0);
        return view;
    }
};
}
