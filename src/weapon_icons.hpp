#pragma once
#include <d2d1_1.h>
#include <wrl/client.h>
#include <initializer_list>
namespace meter {
// Original, code-native silhouettes in a 24 x 24 viewport. No textures/font pack.
inline Microsoft::WRL::ComPtr<ID2D1PathGeometry> createWeaponIcon(ID2D1Factory1* factory, int weapon) {
    Microsoft::WRL::ComPtr<ID2D1PathGeometry> geometry;
    Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
    if (FAILED(factory->CreatePathGeometry(&geometry)) || FAILED(geometry->Open(&sink))) return {};
    const auto polygon = [&](std::initializer_list<D2D1_POINT_2F> points) {
        auto p = points.begin(); if (p == points.end()) return;
        sink->BeginFigure(*p++, D2D1_FIGURE_BEGIN_FILLED);
        for (; p != points.end(); ++p) sink->AddLine(*p);
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    };
    const auto shaft = [&] { polygon({{10,9},{13,9},{13,23},{10,23}}); };
    const auto shield = [&] { polygon({{15,10},{22,10},{22,17},{18.5f,21},{15,17}}); };
    switch (weapon) {
    case 0: polygon({{9,16},{5,8},{8,1},{15,1},{17,8},{13,16}}); shaft(); polygon({{6,15},{17,15},{17,17},{6,17}}); break;
    case 1: polygon({{3,2},{7,5},{7,15},{3,15}}); polygon({{1,14},{10,14},{10,16},{1,16}}); polygon({{4,16},{6,16},{6,23},{4,23}}); shield(); break;
    case 2: polygon({{2,2},{7,4},{10,14},{7,16},{4,12}}); polygon({{7,15},{10,14},{13,22},{10,23}}); polygon({{22,2},{17,4},{14,14},{17,16},{20,12}}); polygon({{14,14},{17,15},{14,23},{11,22}}); break;
    case 3: polygon({{16,1},{17,7},{11,17},{8,16},{14,6}}); polygon({{6,15},{13,18},{12,20},{5,17}}); polygon({{8,18},{10,19},{6,24},{4,23}}); break;
    case 4: polygon({{3,3},{19,3},{21,6},{19,12},{3,12},{1,6}}); shaft(); break;
    case 5: polygon({{9,21},{11,12},{6,10},{3,6},{4,2},{7,5},{11,6},{16,4},{20,5},{22,9},{18,13},{14,12},{12,23}}); break;
    case 6: polygon({{6,1},{10,10},{7,13},{4,10}}); polygon({{6,11},{8,11},{8,23},{6,23}}); shield(); break;
    case 7: polygon({{3,2},{10,2},{10,16},{7,18},{3,15}}); polygon({{5,17},{8,17},{8,23},{5,23}}); shield(); polygon({{10,5},{13,5},{13,12},{10,12}}); break;
    case 8: shaft(); polygon({{11,4},{17,1},{22,3},{22,10},{17,13},{12,10}}); polygon({{9,5},{4,3},{2,7},{5,12},{10,10}}); break;
    case 9: polygon({{3,2},{8,5},{8,14},{4,16},{2,12}}); polygon({{4,14},{7,14},{9,23},{6,23}}); polygon({{15,2},{22,6},{21,15},{16,18},{12,12},{12,6}}); break;
    case 10: polygon({{10,1},{13,1},{13,23},{10,23}}); polygon({{5,2},{10,4},{10,8},{5,7},{2,4}}); polygon({{18,8},{14,10},{14,14},{20,14},{23,11}}); break;
    case 11: polygon({{5,1},{11,4},{16,10},{17,14},{13,21},{7,24},{9,19},{13,13},{11,8}}); polygon({{5,1},{7,1},{7,24},{5,24}}); polygon({{3,11},{22,11},{22,13},{3,13}}); polygon({{19,8},{24,12},{19,16}}); break;
    case 12: polygon({{1,8},{7,8},{7,4},{19,4},{19,7},{24,7},{24,12},{18,12},{18,17},{11,17},{8,13},{5,18},{1,18}}); break;
    case 13: polygon({{1,8},{8,8},{10,5},{16,5},{16,8},{24,8},{24,11},{13,11},{11,14},{8,14},{6,19},{3,18},{4,12},{1,12}}); break;
    default: polygon({{5,4},{19,4},{19,18},{5,18}}); break;
    }
    if (FAILED(sink->Close())) return {};
    return geometry;
}
}
