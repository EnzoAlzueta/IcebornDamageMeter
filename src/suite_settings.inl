// Included in the overlay translation unit so the shared menu uses the same
// live state, render lock and settings writer as the standalone controls.
std::atomic<IcebornOpenSettings> suiteOpen{};
std::atomic<bool> suiteEditing{}, waitForSettingsRelease{};
constexpr IcebornSetting meterOptions[] = {
    {L"Horizontal (%)",0,100,1},{L"Vertical (%)",0,100,1},
    {L"Escala (%)",40,175,5},{L"Opacidad (%)",55,100,5},{L"Ancho",260,640,20},
    {L"Filas",4,16,1},{L"Mostrar daño",0,1,1},{L"Mostrar porcentaje",0,1,1},
    {L"Mostrar DPS",0,1,1},{L"Mostrar participación",0,1,1},{L"Mostrar comparación",0,1,1},
    {L"Mostrar insignia",0,1,1},{L"Mostrar al iniciar la misión",0,1,1},
    {L"Tecla de visibilidad (F1–F12)",1,12,1},{L"Tecla de historial (F1–F12)",1,12,1},
    {L"Tecla de ajustes (F1–F12)",1,12,1},{L"Tecla de movimiento (Ctrl + F1–F12)",1,12,1}
};
int readMeter(int32_t* out,uint32_t count) {
    std::lock_guard lock(renderMutex);
    if (!settingsLoaded || !out || count<std::size(meterOptions)) return 0;
    const auto& c=panel.config;
    const int values[]={int(std::lround(placement.x()*100)),int(std::lround(placement.y()*100)),
        c.scale,c.opacity,c.width,c.maxRows,c.damage,c.share,c.dps,c.time,c.comparison,c.showBadge,c.autoShow,
        c.showKey-VK_F1+1,c.historyKey-VK_F1+1,c.settingsKey-VK_F1+1,c.moveKey-VK_F1+1};
    static_assert(std::size(values)==std::size(meterOptions));
    std::copy(std::begin(values),std::end(values),out);
    return 1;
}
void moveMeterLocked(float x,float y) {
    placement.end();
    placement.load(x,y);
    legacyPlacement=false;
}
int moveMeter(int x,int y) {
    std::lock_guard lock(renderMutex);
    if (!settingsLoaded || x<0 || x>100 || y<0 || y>100) return 0;
    moveMeterLocked(x/100.f,y/100.f);
    refreshRequested=true;
    saveSettings();
    return 1;
}
int setMeter(uint32_t i,int value) {
    std::lock_guard lock(renderMutex);
    if (!settingsLoaded || i>=std::size(meterOptions) || value<meterOptions[i].minimum ||
        value>meterOptions[i].maximum) return 0;
    auto& c=panel.config;
    switch(i) {
    case 0: moveMeterLocked(value/100.f,placement.y()); break;
    case 1: moveMeterLocked(placement.x(),value/100.f); break;
    case 2: c.scale=value; break;
    case 3: c.opacity=value; break;
    case 4: c.width=value; break;
    case 5: c.maxRows=value; break;
    case 6: c.damage=value!=0; break;
    case 7: c.share=value!=0; break;
    case 8: c.dps=value!=0; break;
    case 9: c.time=value!=0; break;
    case 10: c.comparison=value!=0; break;
    case 11: c.showBadge=value!=0; break;
    case 12: c.autoShow=value!=0; break;
    default: {
        int* bindings[]={&c.showKey,&c.historyKey,&c.settingsKey,&c.moveKey};
        const int vk=VK_F1+value-1;
        for(unsigned k=0;k<std::size(bindings);++k)
            if(k!=i-13 && *bindings[k]==vk) return 0;
        *bindings[i-13]=vk;
    }}
    refreshRequested=true;
    saveSettings();
    return 1;
}
const IcebornSettings meterSettings{sizeof(IcebornSettings),ICEBORN_SETTINGS_ABI,L"Medidor de daño",
    uint32_t(std::size(meterOptions)),meterOptions,readMeter,setMeter,moveMeter};

