#pragma once

#include "resource.h"

///////////////////////////////////////////////////////////////////////
//ラベルの矩形を描画する関数
void WM_PAINT_DrawLabels(
    Gdiplus::Graphics& graphics,
    const std::vector<LabelObj>& objs,
    int clientWidth,
    int clientHeight,
    Gdiplus::Font* font
);
///////////////////////////////////////////////////////////////////////
//ドラッグ中の矩形を描画する関数
void WM_PAINT_DrawTmpBox(
    Gdiplus::Graphics& graphics,
    const Gdiplus::RectF& normRect,
    int clientWidth,
    int clientHeight
);
///////////////////////////////////////////////////////////////////////
// WndProc の上部やユーティリティファイルに宣言
void DrawCrosshairLines(HWND hWnd);
///////////////////////////////////////////////////////////////////////
//ラベルのクラス名をポップアップメニューで表示する関数
void ShowClassPopupMenu(HWND hWnd, bool _autoannotation);
///////////////////////////////////////////////////////////////////////
//ラベルのクラス名をポップアップメニューで表示する関数 編集用
int ShowClassPopupMenu_for_Edit(HWND hWnd, ImgObject& _imgobj, int activeObjectIDX, bool _autoannotation);
int ShowClassPopupMenu_for_Edit(HWND hWnd, int activeObjectIDX, bool _autoannotation);
