#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_Anotation2.h"


///////////////////////////////////////////////////
//�R���X�g���N�^
GlobalParams::GlobalParams()
	:IMAGE_EXTENSIONS{ L"*.jpg", L"*.jpeg", L"*.png", L"*.bmp", L"*.gif" }
{
	rect_win.left = 0;
	rect_win.top = 0;
	rect_win.right = 0;
	rect_win.bottom = 0;
	width = 0;
	height = 0;

	imgPaths.clear();
	imgIndex = 0;
}

// �O���[�o���ϐ��̃C���X�^���X���쐬
GlobalParams GP;
