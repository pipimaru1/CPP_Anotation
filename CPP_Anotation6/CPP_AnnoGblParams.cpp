#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_Anotation6.h"

///////////////////////////////////////////////////
//コンストラクタ
GlobalParams::GlobalParams()
	:IMAGE_EXTENSIONS{ L"*.jpg", L"*.jpeg", L"*.png", L"*.bmp", L"*.gif" },
	fontFamily(nullptr), font(nullptr),
	imgIdx(0)
{
	// 画像ファイル関連
	imgObjs.clear(); // 画像ファイルのパスと矩形の配列
	imgIdx = 0;

	/// ウィンドウサイズ
	rect_win.left = 0;
	rect_win.top = 0;
	rect_win.right = 0;
	rect_win.bottom = 0;
	width = 0;
	height = 0;


	//マウス
	//makeBox = false;
	dgMode = DragMode::None; // ドラッグモード
	isMouseMoving = false;
	g_prevPt = { -1, -1 };
    Overlap = 5; // マウスオーバーの裕度

	// 矩形配列
	imgFolderPath.clear();
	//objIdx = 0;

    labelFolderPath = L""; // 現在のラベルフォルダ
    imgFolderPath = L""; // 現在の画像フォルダ

	// クラシフィケーションの初期化 ここを変更することで、クラス名や色を変更できる
	ClsNames = { 
		L"person",
		L"truck",
		L"car",
		L"cat",
		L"dog"
	};
	ClsColors = {
		Gdiplus::Color(255, 255, 0),
		Gdiplus::Color(255, 0, 0),
		Gdiplus::Color(0, 255, 0),
		Gdiplus::Color(0, 0, 255),
		Gdiplus::Color(255, 0, 255)
	};
	ClsDashStyles = {
		Gdiplus::DashStyleSolid,
		Gdiplus::DashStyleDash,
		Gdiplus::DashStyleDot,
		Gdiplus::DashStyleDashDot,
		Gdiplus::DashStyleDashDotDot
	};
	ClsPenWidths ={
		2, 3, 4, 5, 6
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
	Rct.X = 0;
	Rct.Y = 0;
	Rct.Width = 0;
	Rct.Height = 0;
	// クラシフィケーションの初期化
	ClassName.clear();
	ClassNum = 0;
	mOver = false;

	// 描画情報の初期化
	color = Gdiplus::Color(255, 255, 255);
	penWidth = 2;
	dashStyle = Gdiplus::DashStyleSolid;
}

///////////////////////////////////////////////////

ImgObject::ImgObject() = default;
ImgObject::~ImgObject() = default;


