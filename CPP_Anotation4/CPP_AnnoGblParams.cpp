#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_Anotation4.h"


///////////////////////////////////////////////////
//コンストラクタ
GlobalParams::GlobalParams()
	:IMAGE_EXTENSIONS{ L"*.jpg", L"*.jpeg", L"*.png", L"*.bmp", L"*.gif" },
	fontFamily(nullptr), font(nullptr)
{
	/// ウィンドウサイズ
	rect_win.left = 0;
	rect_win.top = 0;
	rect_win.right = 0;
	rect_win.bottom = 0;
	width = 0;
	height = 0;

	// 画像ファイル関連
	imgObjs.clear(); // 画像ファイルのパスと矩形の配列
	imgIdx = 0;

	//マウス
	isDragging = false;
	isMouseMoving = false;
	g_prevPt = { -1, -1 };

	// 矩形配列
	imgFolderPath.clear();
	//objIdx = 0;

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
	ClsColors = {
		Gdiplus::Color(255, 255, 0),
		Gdiplus::Color(255, 0, 0),
		Gdiplus::Color(0, 255, 0),
		Gdiplus::Color(0, 0, 255),
		Gdiplus::Color(255, 0, 255),
		Gdiplus::Color(0, 255, 255),
		Gdiplus::Color(128, 128, 128),
		Gdiplus::Color(128, 0, 128),
		Gdiplus::Color(128, 128, 0),
		Gdiplus::Color(0, 128, 128),
		Gdiplus::Color(192, 192, 192),
		Gdiplus::Color(255, 165, 0)
	};
	ClsDashStyles = {
		Gdiplus::DashStyleSolid,
		Gdiplus::DashStyleDash,
		Gdiplus::DashStyleDot,
		Gdiplus::DashStyleDashDot,
		Gdiplus::DashStyleDashDotDot,
		Gdiplus::DashStyleCustom,
		Gdiplus::DashStyleSolid,
		Gdiplus::DashStyleDash,
		Gdiplus::DashStyleDot,
		Gdiplus::DashStyleDashDot,
		Gdiplus::DashStyleDashDotDot,
		Gdiplus::DashStyleCustom
	};
	ClsPenWidths ={
		2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
	};

	// 選択されたクラシフィケーションのインデックスの初期化
	// なぜ初期値が-1なのか考えよう
	selectedClsIdx = -1; 
}

///////////////////////////////////////////////////
void GlobalParams::InitFont()
{
	fontFamily = new Gdiplus::FontFamily(L"Arial");
	font = new Gdiplus::Font(fontFamily, 12.0f,
		Gdiplus::FontStyleRegular,
		Gdiplus::UnitPixel);
}

void GlobalParams::DestroyFont()
{
	delete font;
	delete fontFamily;
	font = nullptr;
	fontFamily = nullptr;
}

GlobalParams::~GlobalParams()
{
	DestroyFont();
}



///////////////////////////////////////////////////
LabelObj::LabelObj()
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

///////////////////////////////////////////////////

ImgObject::ImgObject() = default;
ImgObject::~ImgObject() = default;


