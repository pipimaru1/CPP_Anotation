#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_Anotation3.h"


///////////////////////////////////////////////////
//コンストラクタ
GlobalParams::GlobalParams()
	:IMAGE_EXTENSIONS{ L"*.jpg", L"*.jpeg", L"*.png", L"*.bmp", L"*.gif" }
{
	/// ウィンドウサイズ
	rect_win.left = 0;
	rect_win.top = 0;
	rect_win.right = 0;
	rect_win.bottom = 0;
	width = 0;
	height = 0;

	// 画像ファイル関連
	imgPaths.clear();
	imgIndex = 0;
	isDragging = false;

	// 矩形配列
	imgFolderPath.clear();
	rectIndex = 0;

	// クラシフィケーションの初期化
	ClsNames = { 
		L"person",
		L"forklift",
		L"tractor",
		L"driver",
		L"truck",
		L"excavator",
		L"wheelloder",
		L"grader",
		L"bulldozer",
		L"pallet",
		L"cargo",
		L"car"
	};
	
	// 選択されたクラシフィケーションのインデックスの初期化
	// なぜ初期値が-1なのか考えよう
	selectedClsIdx = -1; 
}

// グローバル変数のインスタンスを作成
GlobalParams GP;


///////////////////////////////////////////////////
AnnoObject::AnnoObject()
{
	// 矩形の初期化
	rect.X = 0;
	rect.Y = 0;
	rect.Width = 0;
	rect.Height = 0;
	// クラシフィケーションの初期化
	ClassName.clear();
	CalassNum = 0;


	// 描画情報の初期化
	color = Gdiplus::Color(255, 255, 255);
	penWidth = 2;
	dashStyle = Gdiplus::DashStyleSolid;


}