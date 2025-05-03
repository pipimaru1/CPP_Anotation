#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_Anotation2.h"


///////////////////////////////////////////////////
//コンストラクタ
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

// グローバル変数のインスタンスを作成
GlobalParams GP;
