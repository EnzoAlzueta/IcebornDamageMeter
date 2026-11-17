// Included inside the overlay's private namespace. UI work happens at 4 Hz or on interaction.
std::uint64_t utcMilliseconds() {
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}
std::wstring actionNotice;
std::uint64_t actionNoticeUntil=0;
void refreshRows() {
    presentation=panel.build(model,GetTickCount64(),model.serial()==archivedSerial?activeStamp:0);
    if(preview && panel.page==PanelPage::Current) {
        presentation.title=tr(L"PREVIEW · SAMPLE DATA");
        presentation.rowCount=0;
        size_t count=4;
#ifdef METER_DIAGNOSTIC
        if(diagnosticAllWeapons)count=14;
#endif
        const std::wstring names[]={tr(L"You"),std::wstring(tr(L"Hunter"))+L" 2",std::wstring(tr(L"Hunter"))+L" 3",std::wstring(tr(L"Hunter"))+L" 4"};
        const unsigned values[]={24500,21000,14000,10500};const int weapons[]={0,3,11,12};
        for(size_t i=0;i<count;++i) {
            const int weapon=count==14?static_cast<int>(i):weapons[i];
            unsigned damage=count==14?static_cast<unsigned>((14-i)*1000):values[i];
#ifdef METER_DIAGNOSTIC
            if(diagnosticZeroDamage)damage=0;
#endif
            wchar_t value[128]{};const double total=count==14?105000:70000;
            swprintf_s(value,presentation.compact?L"%u · %.1f%% · %.1f/s":L"%u · %.1f%% · %.1f/s · 08:24",damage,100.0*damage/total,damage/504.0);
            presentation.rows[presentation.rowCount++]={count==14?weaponStyle(weapon).name:names[i],value,weapon,
                static_cast<float>(damage)/(count==14?14000:values[0]),i==0,true};
        }
        presentation.columns=presentation.compact?L"":std::wstring(tr(L"Damage"))+L" · % · DPS · "+tr(L"Participation");
        presentation.footer=presentation.compact?L"":L"Ctrl+"+keyName(panel.config.showKey)+tr(L" exits preview");
        presentation.note=presentation.compact?L"":tr(L"Visual example; these values are not saved");
        wchar_t comparison[192]{};
        swprintf_s(comparison,tr(L"Your DPS: %+.1f%% vs last comparable hunt"),8.4);
        presentation.comparison=presentation.compact?L"":std::wstring(comparison)+tr(L" (example)");
        presentation.height=presentation.bodyTop+static_cast<float>(count)*presentation.rowHeight+(presentation.compact?6:78);
    }
    heading=presentation.title;footer=presentation.footer;note=presentation.note;
    if(GetTickCount64()<actionNoticeUntil&&!presentation.compact)footer=actionNotice;
    if(GetTickCount64()<actionNoticeUntil&&presentation.compact)heading=actionNotice;
    if(!preview && panel.page==PanelPage::Current) {
        if(!supported.load())note=tr(L"Checking game version or unsupported version");
        else if(model.phase()==Phase::Live&&!model.dataAvailable())note=tr(L"Team data unavailable · loading or special mode");
    }
    if(presentation.compact){footer.clear();note.clear();}
    rowCount=presentation.rowCount;
    for(size_t i=0;i<rowCount;++i) {
        const auto& r=presentation.rows[i];
        rows[i]={r.name,r.value,r.fraction,r.local,r.weapon,r.colored};
        rows[i].subtitle=r.subtitle;
    }
}
void archiveCurrent(bool interrupted=false) {
    if(!model.serial()||archivedSerial==model.serial())return;
    auto record=captureHunt(model,GetTickCount64(),utcMilliseconds());
    if(interrupted&&model.phase()==Phase::Live){record.partial=true;record.outcome=6;}
    bool any=false;for(const auto& p:record.players)if(p.id)any=true;
    archivedSerial=model.serial();activeStamp=record.timestamp;
    if(!any)return;
    panel.history.add(record);saveHistoryAsync(panel.history,record);
    log("Hunt archived: serial=%llu players=%u duration=%llu ms",archivedSerial,record.partySize,record.durationMs);
}
void processAction(PendingAction command) {
    if(command.action==Action::ShareChat||command.action==Action::CopyDamage) {
        // A missed/transitional memory sample must not invalidate the same
        // cached mission totals currently visible in the panel.
        if(!canShareDamage(model,preview)) {
            actionNotice=preview?tr(L"Exit preview to share real damage"):tr(L"No current damage available to share");
            log("Damage share rejected: preview=%d phase=%d quest=%d total=%llu available=%d",
                preview,static_cast<int>(model.phase()),model.questId(),model.total(),model.dataAvailable());
        } else if(command.action==Action::CopyDamage) {
            actionNotice=copyDamageSummary(window,captureHunt(model,GetTickCount64(),0))?
                tr(L"Summary copied to clipboard"):tr(L"Clipboard busy; try again");
        } else if(damageChatBusy()) {
            actionNotice=tr(L"A send attempt is already in progress");
        } else {
            const auto record=captureHunt(model,GetTickCount64(),0);
            if(beginDamageChat(window,record,panel.config.chatOpenKey)) {
                panel.page=PanelPage::Current;setEditing(false);
                actionNotice=tr(L"Preparing damage summary");
            } else actionNotice=tr(L"Chat busy or unavailable; you can copy the summary");
        }
        actionNoticeUntil=GetTickCount64()+5000;return;
    }
    if(command.action==Action::Close) {
        visibility.hide();preview=false;setEditing(false);return;
    }
    const bool configChanged=panel.change(command.action,command.data);
    if(configChanged)saveSettings();
    if(command.action==Action::Current||command.action==Action::Details||command.action==Action::Last||command.action==Action::SelectRecord)preview=false;
    if(command.action==Action::Current||(command.action==Action::Menu&&panel.page==PanelPage::Current))setEditing(false);
    if(command.action==Action::Move||panel.page==PanelPage::Menu||panel.page==PanelPage::Settings||panel.page==PanelPage::History) {
        if(!visibility.visible())visibility.toggle();
        if(installInput(window))setEditing(true);
    }
}
void poll() {
    settings();
    updateGameLanguage();
    static unsigned previousLanguageRevision=languageRevision.load();
    if(previousLanguageRevision!=languageRevision.load()) {
        previousLanguageRevision=languageRevision.load();refreshRequested=true;
    }
    tickDamageChat();
    ChatResult chatResult;
    if(takeDamageChatResult(chatResult)) {
        actionNotice=chatResult==ChatResult::Completed?tr(L"Check chat: delivery unconfirmed"):
            chatResult==ChatResult::Cancelled?tr(L"Send cancelled; you can copy the summary"):tr(L"Could not send; you can copy the summary");
        actionNoticeUntil=GetTickCount64()+7000;refreshRequested=true;
    }
    bool changed=refreshRequested;refreshRequested=false;
    if(actionNoticeUntil&&GetTickCount64()>=actionNoticeUntil) {
        actionNotice.clear();actionNoticeUntil=0;changed=true;
    }
    if(pendingAction) {processAction(*pendingAction);pendingAction.reset();changed=true;}
    const auto& config=panel.config;
    visibility.automatic(config.autoShow);
    if(GetForegroundWindow()==window&&!damageChatBusy()) {
        if(pressed(config.showKey)) {
            if(GetAsyncKeyState(VK_CONTROL)&0x8000) {preview=!preview;panel.page=PanelPage::Current;}
            else {preview=false;visibility.toggle();if(!visibility.visible())setEditing(false);}
            changed=true;
        }
        if(pressed(config.moveKey)&&(GetAsyncKeyState(VK_CONTROL)&0x8000)) {
            if(installInput(window))setEditing(!isEditing());changed=true;
        }
        if(pressed(config.settingsKey)) {
            if(GetAsyncKeyState(VK_CONTROL)&0x8000)processAction({Action::Menu,0});
            else if(panel.page==PanelPage::Settings) {panel.page=PanelPage::Current;setEditing(false);}
            else processAction({Action::Settings,0});changed=true;
        }
        if(pressed(config.historyKey)) {
            if(panel.page==PanelPage::History) {panel.page=PanelPage::Current;setEditing(false);}
            else processAction({Action::History,0});changed=true;
        }
    } else {keys={};setEditing(false);}
    static bool lastEditing=false;
    if(lastEditing!=isEditing()){lastEditing=isEditing();changed=true;}
    const auto now=GetTickCount64();
    if(now-lastPoll>=250) {
        lastPoll=now;
        static History stored;
        if(takeLoadedHistory(stored)) {
            changed=true;
            if(panel.history.size()==0)panel.history=std::move(stored);
            else {auto recent=panel.history;panel.history=std::move(stored);
                for(size_t i=recent.size();i>0;--i)panel.history.add(*recent.get(i-1));}
        }
        const auto before=model.phase();const auto previousSerial=model.serial();
        auto sample=readSample();
        if(sample.valid&&sample.expedition&&!model.expedition())archiveCurrent(true);
        if(before==Phase::Settling && sample.valid && sample.state==2)archiveCurrent();
        model.update(sample,now);visibility.update(sample);
        if(model.serial()!=previousSerial) {
            if(before==Phase::Live)log("Quest changed without an end state; previous comparison omitted.");
            panel.page=PanelPage::Current;preview=false;setEditing(false);
        }
        if(before!=Phase::Summary&&model.phase()==Phase::Summary)archiveCurrent();
        if((panel.page==PanelPage::Current||panel.page==PanelPage::Details)&&!preview)changed=true;
    }
    scale=static_cast<float>(panel.config.scale)/100;
    const bool nextVisible=preview||isEditing()||visibility.visible();
    if(nextVisible&&!visible)changed=true;
    visible=nextVisible;
    // No row formatting, sorting or layout rebuilding for a hidden panel.
    if(changed && (visible||!font)) {refreshRows();refreshLayouts();}
}
