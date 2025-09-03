#pragma once

#include "resource.h"

///////////////////////////////////////////////////////////////////////
//���x���̋�`��`�悷��֐�
void WM_PAINT_DrawLabels(
    Gdiplus::Graphics& graphics,
    const std::vector<LabelObj>& objs,
    int clientWidth,
    int clientHeight,
    Gdiplus::Font* font
);
///////////////////////////////////////////////////////////////////////
//�h���b�O���̋�`��`�悷��֐�
void WM_PAINT_DrawTmpBox(
    Gdiplus::Graphics& graphics,
    const Gdiplus::RectF& normRect,
    int clientWidth,
    int clientHeight
);
///////////////////////////////////////////////////////////////////////
// WndProc �̏㕔�⃆�[�e�B���e�B�t�@�C���ɐ錾
void DrawCrosshairLines(HWND hWnd);
///////////////////////////////////////////////////////////////////////
//���x���̃N���X�����|�b�v�A�b�v���j���[�ŕ\������֐�
void ShowClassPopupMenu(HWND hWnd, bool _autoannotation);
///////////////////////////////////////////////////////////////////////
//���x���̃N���X�����|�b�v�A�b�v���j���[�ŕ\������֐� �ҏW�p
int ShowClassPopupMenu_for_Edit(HWND hWnd, ImgObject& _imgobj, int activeObjectIDX, bool _autoannotation);
int ShowClassPopupMenu_for_Edit(HWND hWnd, int activeObjectIDX, bool _autoannotation);
