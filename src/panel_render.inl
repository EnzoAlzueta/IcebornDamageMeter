// Included in the private overlay namespace after the text drawing helper.
void pointerInteraction(float width,float height) {
    static bool wasDown=false;
    if(GetForegroundWindow()!=window) {wasDown=false;if(placement.end())saveSettings();return;}
    POINT cursor{};RECT client{};bool down=(GetAsyncKeyState(VK_LBUTTON)&0x8000)!=0;
    const bool edit=getEditPointer(cursor,down);
    if((!edit&&(!GetCursorPos(&cursor)||!ScreenToClient(window,&cursor)))||!GetClientRect(window,&client)||client.right<=0||client.bottom<=0)return;
    const float px=cursor.x*screenWidth/client.right,py=cursor.y*screenHeight/client.bottom;
    CURSORINFO info{sizeof(CURSORINFO)};
    const bool interactive=isEditing()||(GetCursorInfo(&info)&&(info.flags&CURSOR_SHOWING));
    if(down&&!wasDown&&interactive) {
        if(!visible&&badgeRect.contains(px,py)){visibility.toggle();refreshRequested=true;}
        else if(visible) {
            const float fit=width/panel.config.width;
            const float lx=(px-panelRect.x)/fit,ly=(py-panelRect.y)/fit;
            bool hit=false;
            for(size_t i=0;i<presentation.buttonCount;++i) {
                const auto& button=presentation.buttons[i];
                if(button.rect.contains(lx,ly)){pendingAction=PendingAction{button.action,button.data};hit=true;break;}
            }
            if(!hit&&isEditing()&&ly>=0&&ly<(presentation.compact?20:32)&&lx>=0&&lx<panel.config.width)placement.begin(px,py,panelRect);
        }
    }
    if(isEditing()&&placement.dragging()&&down){
        placement.move(px,py,screenWidth,screenHeight,width,height);
        panelRect=placement.bounds(screenWidth,screenHeight,width,height);
    }
    if((!down||!isEditing())&&placement.end())saveSettings();
    wasDown=down;
    const auto hotspot=visible?panelRect:badgeRect;
    const RECT inputRect{static_cast<LONG>(hotspot.x*client.right/screenWidth),static_cast<LONG>(hotspot.y*client.bottom/screenHeight),
        static_cast<LONG>((hotspot.x+hotspot.width)*client.right/screenWidth),static_cast<LONG>((hotspot.y+hotspot.height)*client.bottom/screenHeight)};
    setInputHotspot(inputRect);
}
void drawPanelContents() {
    const float width=static_cast<float>(panel.config.width),height=presentation.height;
    const float opacity=static_cast<float>(panel.config.opacity)/100;
    brush->SetColor(D2D1::ColorF(0x111612,opacity));
    context2d->FillRectangle(D2D1::RectF(0,0,width,height),brush.Get());
    brush->SetColor(D2D1::ColorF(0x36392f,opacity));
    context2d->FillRectangle(D2D1::RectF(2,2,width-2,presentation.compact?20.0f:32.0f),brush.Get());
    brush->SetColor(D2D1::ColorF(0x8c805d,0.92f));
    context2d->DrawRectangle(D2D1::RectF(0.5f,0.5f,width-0.5f,height-0.5f),brush.Get(),1);
    if(!presentation.compact) {
      brush->SetColor(D2D1::ColorF(0x403e2e,0.9f));
      context2d->DrawRectangle(D2D1::RectF(3,3,width-3,height-3),brush.Get(),1);
      brush->SetColor(D2D1::ColorF(0xc4af75));
      for(int cornerIndex=0;cornerIndex<4;++cornerIndex) {
        const float x=(cornerIndex&1)?width-1:1,y=(cornerIndex&2)?height-1:1;
        context2d->DrawLine(D2D1::Point2F(x,y),D2D1::Point2F(x+((cornerIndex&1)?-11:11),y),brush.Get(),2);
        context2d->DrawLine(D2D1::Point2F(x,y),D2D1::Point2F(x,y+((cornerIndex&2)?-11:11)),brush.Get(),2);
      }
    }
    text(headingLayout.Get(),12,presentation.compact?3.0f:9.0f,0xe2c977);
    for(size_t i=0;i<presentation.buttonCount;++i) {
        const auto& b=presentation.buttons[i];
        if(b.text.empty())continue;
        const auto r=D2D1::RectF(b.rect.x,b.rect.y,b.rect.x+b.rect.width,b.rect.y+b.rect.height);
        brush->SetColor(D2D1::ColorF(b.selected?0x4b4932:0x252b23,0.9f));
        context2d->FillRectangle(r,brush.Get());
        if(b.selected) {
            brush->SetColor(D2D1::ColorF(0xd5b961));
            context2d->DrawLine(D2D1::Point2F(r.left+3,r.bottom-1),D2D1::Point2F(r.right-3,r.bottom-1),brush.Get(),1);
        }
        text(buttonLayouts[i].Get(),b.rect.x+4,b.rect.y+4,b.selected?0xf1d47a:0xc9c9b5);
    }
    const auto geometry=rowGeometry(presentation,width);
    if(!presentation.compact)text(columnsLayout.Get(),presentation.narrow?18:geometry.valueX,presentation.bodyTop-18,0xb6b29b);
    for(size_t i=0;i<rowCount;++i) {
        const float top=presentation.bodyTop+presentation.rowHeight*static_cast<float>(i);
        const float rowHeight=presentation.rowHeight-3;
        if(rows[i].colored) {
            const auto color=weaponStyle(rows[i].weapon).color;
            brush->SetColor(D2D1::ColorF(color,0.24f));
            context2d->FillRectangle(D2D1::RectF(10,top,width-10,top+rowHeight),brush.Get());
            brush->SetColor(D2D1::ColorF(color,0.88f));
            context2d->FillRectangle(D2D1::RectF(10,top,13,top+rowHeight),brush.Get());
            brush->SetColor(D2D1::ColorF(color,0.60f));
            context2d->FillRectangle(D2D1::RectF(10,top,10+(width-20)*rows[i].fraction,top+rowHeight),brush.Get());
        } else {
            brush->SetColor(D2D1::ColorF(i%2?0x232a22:0x1b221b,0.7f));
            context2d->FillRectangle(D2D1::RectF(10,top,width-10,top+rowHeight),brush.Get());
        }
        if(rows[i].local) {
            brush->SetColor(D2D1::ColorF(0xe5cf87));
            context2d->FillRectangle(D2D1::RectF(10,top,12,top+rowHeight),brush.Get());
        }
        if(!presentation.settings&&!presentation.menu) {
            if(validWeapon(rows[i].weapon)&&gameWeaponAtlas) {
                const float column=static_cast<float>(WeaponAtlasColumns[static_cast<size_t>(rows[i].weapon)]);
                context2d->DrawBitmap(gameWeaponAtlas.Get(),D2D1::RectF(16,top,40,top+24),1.0f,
                    D2D1_INTERPOLATION_MODE_LINEAR,D2D1::RectF(column*64,0,(column+1)*64,64),nullptr);
            } else if(unknownWeaponIcon) {
                context2d->SetTransform(D2D1::Matrix3x2F::Scale(0.75f,0.75f)*D2D1::Matrix3x2F::Translation(18,top+2.5f));
                brush->SetColor(D2D1::ColorF(0xe6e4d2));context2d->FillGeometry(unknownWeaponIcon.Get(),brush.Get());
                context2d->SetTransform(D2D1::Matrix3x2F::Identity());
            }
        }
        text(rows[i].nameLayout.Get(),geometry.nameX,top+geometry.nameY,0xe5e5d5);
        text(rows[i].valueLayout.Get(),geometry.valueX,top+geometry.valueY,0xf0eedc);
        text(rows[i].subtitleLayout.Get(),geometry.subtitleX,top+geometry.subtitleY,0xb6b29b);
    }
    // Settings +/- buttons are above the row fills, so redraw just those.
    if(presentation.settings)for(size_t i=0;i<presentation.buttonCount;++i) {
        const auto& b=presentation.buttons[i];
        if(b.action!=Action::Increase&&b.action!=Action::Decrease)continue;
        brush->SetColor(D2D1::ColorF(0x3c4231,0.95f));
        context2d->FillRectangle(D2D1::RectF(b.rect.x,b.rect.y,b.rect.x+b.rect.width,b.rect.y+b.rect.height),brush.Get());
        text(buttonLayouts[i].Get(),b.rect.x+6,b.rect.y+3,0xe2c977);
    }
    if(!presentation.compact) {
    const float footY=presentation.bodyTop+presentation.rowHeight*static_cast<float>(rowCount)+8;
    brush->SetColor(D2D1::ColorF(0x706548,0.8f));
    context2d->DrawLine(D2D1::Point2F(12,footY-5),D2D1::Point2F(width-12,footY-5),brush.Get(),1);
    text(footerLayout.Get(),12,footY,0xd1c6a1);
    text(noteLayout.Get(),12,footY+18,0xa6ad9e);
    text(comparisonLayout.Get(),12,footY+36,0xd1bc7b);
    }
}
bool rebuildPanelCache() {
    ComPtr<ID2D1CommandList> commands;
    if(!checked(context2d->CreateCommandList(&commands),"Create panel cache"))return false;
    context2d->SetTarget(commands.Get());context2d->BeginDraw();
    context2d->SetTransform(D2D1::Matrix3x2F::Identity());drawPanelContents();
    const auto result=context2d->EndDraw();context2d->SetTarget(nullptr);
    if(!checked(result,"Record panel cache")||!checked(commands->Close(),"Close panel cache"))return false;
    panelCommands=commands;sceneDirty=false;
#ifdef METER_DIAGNOSTIC
    ++cacheBuilds;
#endif
    return true;
}
void paint(UINT index,const IcebornPreviewArea* area=nullptr) {
    const auto size=targets[index]->GetSize();if(size.width<100||size.height<100)return;
    screenWidth=size.width;screenHeight=size.height;
    const float width=static_cast<float>(panel.config.width),height=presentation.height;
    const float fit=(std::min)({scale,(size.width-16)/width,(size.height-16)/height});
    if(legacyPlacement && !area) {
        const bool right=corner==1||corner==2,bottom=corner==2||corner==3;
        placement.load((right?size.width-width*fit-12:12)/size.width,(bottom?size.height-height*fit-12:12)/size.height);
        legacyPlacement=false;
    }
    panelRect=placement.bounds(size.width,size.height,width*fit,height*fit);
    const auto badge=placement.bounds(size.width,size.height,36,24);badgeRect={badge.x,badge.y,36,24};
    if(area) {
        if(area->x>=0){
            panelRect.x=std::clamp(size.width*area->x/100.f,0.f,size.width-panelRect.width);
            panelRect.y=std::clamp(size.height*area->y/100.f,0.f,size.height-panelRect.height);
        }
        panelRect.y=area->displayTop(panelRect.x,panelRect.y,panelRect.width,panelRect.height);
    } else pointerInteraction(width*fit,height*fit);
    const bool contentChanged=sceneDirty||!panelCommands;
    if(visible && contentChanged && !rebuildPanelCache())return;
    static bool previousVisible=false;
    static Rect previousRect{};
    const auto currentRect=visible?panelRect:badgeRect;
    const bool samePlacement=currentRect.x==previousRect.x&&currentRect.y==previousRect.y&&
        currentRect.width==previousRect.width&&currentRect.height==previousRect.height;
    const bool reusable=gameDevice11&&(!visible||!contentChanged)&&(area||!isEditing())&&previousVisible==visible&&samePlacement;
    previousVisible=visible;previousRect=currentRect;
    if(reusable) {
        const auto reused=dx11.reuse();
        if(reused!=S_FALSE){checked(reused,"DX11 reuse panel pixels");return;}
    }
    auto resource=wrapped[index].Get();
    if(gameDevice11) {
        const auto begin=dx11.begin();
        if(begin==S_FALSE){checked(dx11.composite(),"DX11 pending composition");return;}
        if(!checked(begin,"DX11 acquire UI texture"))return;
    } else on12->AcquireWrappedResources(&resource,1);
    context2d->SetTarget(targets[index].Get());context2d->BeginDraw();
    auto bounds=visible?D2D1::RectF(panelRect.x-2,panelRect.y-2,panelRect.x+panelRect.width+2,panelRect.y+panelRect.height+2):
        D2D1::RectF(badgeRect.x-2,badgeRect.y-2,badgeRect.x+badgeRect.width+2,badgeRect.y+badgeRect.height+2);
    POINT cursor{};bool down=false;RECT client{};
    const bool showPointer=!area&&getEditPointer(cursor,down)&&GetClientRect(window,&client)&&client.right>0&&client.bottom>0;
    const float pointerX=showPointer?cursor.x*size.width/client.right:0,pointerY=showPointer?cursor.y*size.height/client.bottom:0;
    if(showPointer){bounds.left=(std::min)(bounds.left,pointerX-4);bounds.top=(std::min)(bounds.top,pointerY-4);
        bounds.right=(std::max)(bounds.right,pointerX+20);bounds.bottom=(std::max)(bounds.bottom,pointerY+26);}
    if(gameDevice11) {
        // Match the DX11 scissor so fractional placements cannot expose stale pixels.
        bounds={std::floor(bounds.left),std::floor(bounds.top),std::ceil(bounds.right),std::ceil(bounds.bottom)};
        context2d->SetTransform(D2D1::Matrix3x2F::Identity());
        context2d->PushAxisAlignedClip(bounds,D2D1_ANTIALIAS_MODE_ALIASED);
        context2d->Clear(D2D1::ColorF(0,0.0f));context2d->PopAxisAlignedClip();
    }
    if(!visible) {
        context2d->SetTransform(D2D1::Matrix3x2F::Translation(badgeRect.x,badgeRect.y));
        brush->SetColor(D2D1::ColorF(0x272c21,0.88f));context2d->FillRectangle(D2D1::RectF(0,0,36,24),brush.Get());
        brush->SetColor(D2D1::ColorF(0x9c8a55));context2d->DrawRectangle(D2D1::RectF(0.5f,0.5f,35.5f,23.5f),brush.Get(),1);
        text(badgeLayout.Get(),7,4,0xe2c977);
    } else {
        context2d->SetTransform(D2D1::Matrix3x2F::Scale(fit,fit)*D2D1::Matrix3x2F::Translation(panelRect.x,panelRect.y));
        context2d->DrawImage(panelCommands.Get());
        if(!area&&isEditing()) {
            brush->SetColor(D2D1::ColorF(0xc4b77a));
            context2d->DrawRectangle(D2D1::RectF(1,1,width-1,height-1),brush.Get(),1.5f);
        }
    }
    if(showPointer) {
        context2d->SetTransform(D2D1::Matrix3x2F::Translation(pointerX,pointerY));
        const D2D1_POINT_2F points[]={{0,0},{0,18},{5,14},{9,22},{12,20},{8,12},{16,12}};
        brush->SetColor(D2D1::ColorF(0x0b1018));
        for(size_t i=0;i<std::size(points);++i)context2d->DrawLine(points[i],points[(i+1)%std::size(points)],brush.Get(),3);
        brush->SetColor(D2D1::ColorF(0xf4eac2));
        for(size_t i=0;i<std::size(points);++i)context2d->DrawLine(points[i],points[(i+1)%std::size(points)],brush.Get(),1.5f);
    }
    const auto result=context2d->EndDraw();context2d->SetTarget(nullptr);
    if(gameDevice11)checked(dx11.end(bounds),"DX11 compose panel");
    else {on12->ReleaseWrappedResources(&resource,1);context11->Flush();}
    checked(result,"Draw cached panel");
}
