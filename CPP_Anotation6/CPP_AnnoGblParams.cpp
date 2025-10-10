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
	imgIdxCompare = 0; // 前後画像の比較用インデックス

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

	//OnnxPath = L".\\yolov5s.onnx";
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


//Windowsの音
wchar_t SOUND_ARRY[39][2][256] = {
	{L"NFP 完了",                          L"Notification.Proximity" },
	{L"NFP 接続",                          L"ProximityConnection" },
	{L"Windows の終了",                    L"SystemExit" },
	{L"Windows の起動",                    L"SystemStart" },
	{L"Windows テーマの変更",              L"ChangeTheme" },
	{L"Windows ユーザー アカウント制御",   L"WindowsUAC" },
	{L"Windows ログオフ",                  L"WindowsLogoff" },
	{L"Windows ログオン",                  L"WindowsLogon" },
	{L"インスタント メッセージの通知",     L"Notification.IM" },
	{L"システム エラー",                   L"SystemHand" },
	{L"システム通知",                      L"SystemNotification" },
	{L"ツール バー バンドの表示",          L"ShowBand" },
	{L"デスクトップ メールの通知",         L"MailBeep" },
	{L"デバイスの切断",                    L"DeviceDisconnect" },
	{L"デバイスの接続",                    L"DeviceConnect" },
	{L"デバイスの接続の失敗",              L"DeviceFail" },
	{L"バッテリ低下アラーム",              L"LowBatteryAlarm" },
	{L"バッテリ切れアラーム",              L"CriticalBatteryAlarm" },
	{L"プログラム エラー",                 L"AppGPFault" },
	{L"プログラムの終了",                  L"Close" },
	{L"プログラムの起動",                  L"Open" },
	{L"メッセージ (問い合わせ)",           L"SystemQuestion" },
	{L"メッセージ (情報)",                 L"SystemAsterisk" },
	{L"メッセージ (警告)",                 L"SystemExclamation" },
	{L"メッセージのシェイク",              L"MessageNudge" },
	{L"メニュー コマンド",                 L"MenuCommand" },
	{L"メニュー ポップアップ",             L"MenuPopup" },
	{L"一般の警告音",                      L".Default" },
	{L"予定表のアラーム",                  L"Notification.Reminder" },
	{L"元に戻す (拡大)",                   L"RestoreUp" },
	{L"元に戻す (縮小)",                   L"RestoreDown" },
	{L"印刷完了",                          L"PrintComplete" },
	{L"新着テキスト メッセージの通知",     L"Notification.SMS" },
	{L"新着ファックスの通知",              L"FaxBeep" },
	{L"新着メールの通知",                  L"Notification.Mail" },
	{L"最大化",                            L"Maximize" },
	{L"最小化",                            L"Minimize" },
	{L"通知",                              L"Notification.Default" },
	{L"選択",                              L"CCSelect" }
};