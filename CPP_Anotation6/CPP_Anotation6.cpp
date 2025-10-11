    // CPP_Anotation.cpp : アプリケーションのエントリ ポイントを定義します。

#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFunctions.h"
#include "CPP_Menu.h"
#include "CPP_Anotation6.h"

#include "CPP_YoloAuto.h"
#include "Sliderbox.hpp"

// 必要なライブラリ
// 修正: cv::imread関数に渡すパスをstd::stringに変換する必要があります。
// cv::imreadはstd::string型の引数を受け取るため、std::wstringをstd::stringに変換します。
//#include <opencv2/opencv.hpp>
//#include <string>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")
//#pragma comment(lib, "Shlwapi.lib")

#define MAX_LOADSTRING 100

struct GdiplusRAII {
    ULONG_PTR token{ 0 };
    GdiplusRAII() {
        Gdiplus::GdiplusStartupInput in;
        Gdiplus::GdiplusStartup(&token, &in, nullptr);
    }
    ~GdiplusRAII() {
        Gdiplus::GdiplusShutdown(token);   // ここが最後に呼ばれる
    }
};

//GDIPlusのスタート
GdiplusRAII gdi;

// グローバル変数のインスタンスを作成
GlobalParams GP;

std::vector<MenuItemOnnx> g_menu_onnx;

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

///////////////////////////////////////////////////////////////
// このコード モジュールに含まれる関数の宣言
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
ULONG_PTR g_GdiToken;

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。
    
    //　コマンドライン引数
	// Cpp_Anotation.exe <分類ファイル> <画像フォルダ> <ラベルフォルダ> [<onnxファイル>] [onnxサイズ:640/1280/1920]

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool isInitOK = false;

    // デフォルトのままだと lpCmdLine はクォート含まれないので argv の方を使う
    if (argc >= 4) {
        std::wstring classFile = argv[1];   // クラス名ファイル
		std::wstring imageFolder = argv[2]; // 画像フォルダ
		std::wstring labelFolder = argv[3]; // ラベルフォルダ

        // ファイル・フォルダ存在確認
        if (PathFileExistsW(classFile.c_str()) &&
            PathIsDirectoryW(imageFolder.c_str()) &&
            PathIsDirectoryW(labelFolder.c_str()))
        {
            // クラスファイル読み込み
            LoadClassification(classFile, GP.ClsNames, GP.ClsColors, GP.ClsDashStyles, GP.ClsPenWidths, 0);
            // 画像とラベルのパスを保存
            GP.imgFolderPath = imageFolder;
            GP.labelFolderPath = labelFolder;
            // 画像読み込み
            LoadImageFilesMP(GP.imgFolderPath, GP.imgObjs);
            // ラベル読み込み
            LoadLabelFilesMP(GP.imgObjs, GP.labelFolderPath, L".txt", 1);
            isInitOK = true;
        }
    }
    if (argc >= 5)
    {
        std::wstring _onnxfile = argv[4]; // ラベルフォルダ
        if (PathFileExistsW(_onnxfile.c_str()))
        {
            GDNNP.gOnnxPath = _onnxfile;
            isInitOK = true;
        }
    }
    if (argc >= 6)
    { 
        int _onnxsize = _wtoi(argv[5]); // ラベルフォルダ
        if (_onnxsize == 640 || _onnxsize == 1280 || _onnxsize == 1920)
        {
            GDNNP.yolo.inputW = _onnxsize;
            GDNNP.yolo.inputH = _onnxsize;
            isInitOK = true;
        }
	}


    // GDIPlusのスタートの後に フォントを生成
    GP.InitFont();

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CPPANOTATION, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    // ファイルやフォルダが不正だった場合はエラーメッセージ
    if (!isInitOK && (argc != 1)) 
    {
        MessageBoxW(NULL,
            L"コマンドライン引数が不足しているか、ファイル／フォルダが存在しません。\n"
            L"引数: <分類ファイル> <画像フォルダ> <ラベルフォルダ>",
            L"エラー",
            MB_OK | MB_ICONERROR);
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CPPANOTATION));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    // 最後に終了処理
    GP.DestroyFont();
    //GDIをシャットダウンする前に画像オブジェクトをクリアする
	//あえて呼びだしている。デストラクタが呼ばれるので書かなくてもいいが、書かない場合はGPとGDI両方クリアする。
    
    GP.imgObjs.clear();
    GdiplusShutdown(g_GdiToken);

    return (int)msg.wParam;
}

///////////////////////////////////////////////////////
//  関数: MyRegisterClass()
//  目的: ウィンドウ クラスを登録します。
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CPPANOTATION));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CPPANOTATION);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

///////////////////////////////////////////////////////
//   関数: InitInstance(HINSTANCE, int)
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}
/////////////////////////////////////////////////////////////////////////
// 課題
// クラシフィケーションをファイルから読み込む機能を追加してください。
// 画像やラベルのフォルダバスを保存する機能を追加してください。
// ⇒レジストリに保存する機能を追加済 参考にしてください。
// 画像送りでファイルを更新する機能を追加してください。
 
/////////////////////////////////////////////////////////////////////////
// 上級課題
// アノテーションデータをSQLのテーブルに保存していく
// 画像データをbase64に変換し、SQLのテーブルに保存していく
// 画像データとアノテーションは紐づけて保存していく
// アノテーションの作業手順を想定し、ユーザーインターフェースやソフトウェアの構造を考える

/////////////////////////////////////////////////////////////////////////
// 画像フォルダのパス
// フィルパスのフォルダは'/'で区切る必要があります。
// '\'で記述する場合は'\\'に置き換える必要があります。
const wchar_t* INIT_IMGFOLDER = L"../images/";  // JPEGまたはPNG

///////////////////////////////////////////////////////////////////////
//タイトルバーに画像のパスを表示
void SetStringToTitlleBar(HWND hWnd, std::wstring _imgfolder, std::wstring _labelfolder, int _Idx, int _Total)
{
	std::wstring title = 
        L"Annotation Tool - " + _imgfolder + L" - " + _labelfolder + 
        L" [ " + std::to_wstring(_Idx) + L" / " + std::to_wstring(_Total) + L"]";
    SetWindowText(hWnd, title.c_str());
    return;
}

/////////////////////////////////////////////////////////////////////////
void CheckMenu(HWND hWnd, int _IDM, bool _sw);

//////////////////////////////////////////////////////////////////////////
void set_onnx_files_in_menu(int _num)
{ 
	if (_num < g_menu_onnx.size())
    {
		GP.select_onnx_num = _num; //メニューにチェック入れるため
        GDNNP.gOnnxPath = g_menu_onnx[_num].OnnxPath; // ファイルパスを保存
        GDNNP.yolo.YoloVersion = g_menu_onnx[_num].YoloType; // YOLOバージョンを保存
        GDNNP.yolo.inputW = g_menu_onnx[_num].Resolution; // 入力画像幅を保存
        GDNNP.yolo.inputH = g_menu_onnx[_num].Resolution; // 入力画像幅を保存

        //ファイルの存在確認
        //if (PathFileExistsW(g_menu_onnx[_num].ClsNamePath.c_str()))
		if (std::filesystem::exists(g_menu_onnx[_num].ClsNamePath))
        {
            LoadClassification(g_menu_onnx[_num].ClsNamePath, GP.ClsNames, GP.ClsColors, GP.ClsDashStyles, GP.ClsPenWidths, 0);//0=読み込み、1=書き込み
        }else{
            MessageBox(NULL, (g_menu_onnx[_num].ClsNamePath + L"\n(クラス定義ファイルがありません)").c_str(),
                L"警告", MB_OK | MB_ICONWARNING);
		}
    }
}

///////////////////////////////////////////////////////////////////////
// メニューのチェック状態を更新する関数
int CreatePopupMenuFor_Labels_in_CurrentImage(HWND hWnd);

/////////////////////////////////////////////////////////////////////////
//未ラベルの画像までジャンプする関数
void JumpToUnlabeledImage(HWND hWnd);

/////////////////////////////////////////////////////////////////////////
// アノテーションデータを保存する関数 WM_Procの補助関数
// _scは0,25,50,75,100のいずれかで、0だとスケールしない
//int  SaveAnnotations(HWND hWnd, int _sc);

/////////////////////////////////////////////////////////////////////////
// 自動アノテーション確定
// 「Alt+数字」キーのハンドリング　ヘルパ関数（確定処理本体）
static void AutoConfirmByKey(int keyIdx /*0..4*/)
{
    if (keyIdx < 0 || keyIdx >= 8) 
        return;

    const float th = GP.autoConfirmThresh[keyIdx];
    //const int   _targetClass = keyIdx;            // 1→クラス0, 2→クラス1 ... の既存マッピングに合わせる
    ImgObject& _cur = GP.imgObjs[GP.imgIdx];

    size_t _before = _cur.objs.size();
    for (auto& p : AutoDetctedObjs.objs) {
        // クラス一致 & Confが閾値以上 → 確定（書き込み）
        if(!p.isAutoConfirmed)
        {
            if (p.Conf >= th) {
                LabelObj o = p;
                // 必要なら最小サイズ補正や正規化をここで
                // NormalizeRect(o.Rct);
                // FixLabelBox(o, minW, minH) を使うならここで呼ぶ

				p.isAutoConfirmed = true; // 自動アノテーション済みフラグを立てる このフラグもコピーされるが構わないのかな
                _cur.objs.push_back(std::move(o));
            }
        }
    }

    if (_cur.objs.size() != _before) {
        _cur.isEdited = true;
        // 再描画
        // InvalidateRect(hWnd, NULL, TRUE); ← 呼び元でやる
        // サウンド
        // PlaySound(...); ← 既存の通知音があれば流用
    }
}

/////////////////////////////////////////////////////////////////////////
// Undo機能の実装
// グローバル or 静的
//static std::vector<LabelObj> g_lastCleared;
//static bool g_hasUndo = false;

// クリア前に保存
//g_lastCleared = cur.objs;
//g_hasUndo = true;

// Undo ハンドラ（Ctrl+Zに割当）
static void UndoClear()
{
    if (!GP.undo.hasUndo || GP.imgObjs.empty()) 
        return;
    if (GP.undo.imgIdxUndo == GP.imgIdx)
    {
        auto& cur = GP.imgObjs[GP.imgIdx];
        cur.objs = GP.undo.lastCleared;
        cur.isEdited = true;
        GP.undo.hasUndo = false;
        //InvalidateRect(hWnd, nullptr, TRUE);
    }
    return; // 画像が変わっていたらUndoしない
}

/////////////////////////////////////////////////////////////////////////
// どこか共通のヘルパ（任意）
//static void ClearCurrentImageLabels(HWND hWnd)
//{
//    if (GP.imgObjs.empty()) return;
//    auto& cur = GP.imgObjs[GP.imgIdx];
//    if (!cur.objs.empty()) {
//        cur.objs.clear();          // ラベルを全消去
//        cur.isEdited = true;       // 編集フラグON（後で保存判定に使えます）:contentReference[oaicite:0]{index=0}
//        InvalidateRect(hWnd, nullptr, TRUE);  // 再描画
//        // 必要なら通知音：
//        // PlaySound(SOUND_ARRY[37][1], NULL, SND_ALIAS | SND_ASYNC | SND_NODEFAULT);
//    }
//}

/////////////////////////////////////////////////////////////////////////
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//  目的: メイン ウィンドウのメッセージを処理します。
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//  WM_SIZE    - ウィンドウのサイズ変更時の処理
//  WM_CREATE  - ウィンドウの作成時に画像を読み込む
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        if (GP.imgFolderPath.empty()) {
            GP.imgObjs.clear(); // 画像ファイルのパスと矩形の配列
            GP.imgFolderPath = INIT_IMGFOLDER;
            LoadImageFiles(GP.imgFolderPath, GP.imgObjs);
        }
        CheckMenues(hWnd);
        SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, 0, (int)GP.imgObjs.size()); // タイトルバーに画像のパスを表示

		//iniファイルからONNXメニューを読み込む
        std::wstring _inifile = L"default.ini";
		//ファイルの存在確認
		//if (PathFileExistsW(_inifile.c_str()))
        if (std::filesystem::exists(_inifile))
        {
            loadmenu_onnx(_inifile, g_menu_onnx);
            make_onnx_menus_by_id(hWnd, IDM_ONNX000, g_menu_onnx, IDM_ONNX000, L"ONNXファイル選択");
        }

    }
    break;
    case WM_INITMENUPOPUP:
	{	
        // ポップアップメニューの初期化
		HMENU hMenu = GetMenu(hWnd);
        CheckMenu(hWnd, IDM_NOSAVE_SMALL00, (GP.minimumLabelSize == 0.0f)); // メニューのチェック状態を更新
        CheckMenu(hWnd, IDM_NOSAVE_SMALL010, (GP.minimumLabelSize == 0.01f)); // メニューのチェック状態を更新
        CheckMenu(hWnd, IDM_NOSAVE_SMALL015, (GP.minimumLabelSize == 0.015f)); // メニューのチェック状態を更新
        CheckMenu(hWnd, IDM_NOSAVE_SMALL02, (GP.minimumLabelSize == 0.02f)); // メニューのチェック状態を更新
        CheckMenu(hWnd, IDM_NOSAVE_SMALL03, (GP.minimumLabelSize == 0.03f)); // メニューのチェック状態を更新
        CheckMenu(hWnd, IDM_NOSAVE_SMALL05, (GP.minimumLabelSize == 0.05f)); // メニューのチェック状態を更新
        CheckMenu(hWnd, IDM_NOSAVE_SMALL10, (GP.minimumLabelSize == 0.1f)); // メニューのチェック状態を更新
        CheckMenu(hWnd, IDM_NOSAVE_OR_CORRECT, GP.isMinimumLabelCrrect); // メニューのチェック状態を更新
        // メニュー表示を更新
        DrawMenuBar(hWnd);
    }
    break;

    ///////////////////////////////////////////////////////////
	//
    // WM_COMMAND メッセージの処理
    // 
    ///////////////////////////////////////////////////////////
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        // 動的メニューIDの範囲判定（例: IDM_PMENU_CLSNAME00～）
        const UINT BASE_ID = IDM_PMENU_CLSNAME00;
        size_t clsCount = GP.ClsNames.size();
        if (wmId >= BASE_ID && wmId < BASE_ID + clsCount) 
        {
            GP.selectedClsIdx = wmId - BASE_ID;
            break; // 処理したので switch を抜ける
        }

        // 選択されたメニューの解析:
        switch (wmId)
        {
            case IDM_LOAD_LABELS:
		    {
			    // ファイルオープンダイアログを表示
                std::wstring _folderpath;
                _folderpath = GetFolderPathIFR(hWnd, L"読込ラベルフォルダを選択してください"); // フォルダ選択ダイアログを表示
                //_folderpath = GetFolderPathEx(hWnd, L"読込ラベルフォルダを選択してください"); // フォルダ選択ダイアログを表示

                // フォルダ選択ダイアログを表示
                if (!_folderpath.empty()) 
                {
                    GP.labelFolderPath = _folderpath; // フォルダパスを指定

                    // フォルダが選択された場合、アノテーションデータを読み込み
                    //LoadLabelFiles(GP.imgObjs, GP.labelFolderPath, L".txt", 1);
                    LoadLabelFilesMP(GP.imgObjs, GP.labelFolderPath, L".txt", 1);

                    //タイトルバーに編集中の画像とラベルのパスを表示
                    SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.imgIdx, (int)GP.imgObjs.size()); // タイトルバーに画像のパスを表示
                }
		    }
		    break;
            case IDM_SAVE_LABELS:
		    {
			    SaveAnnotations(hWnd, L"書込ラベルフォルダを選択してください", 1.0f); // アノテーションデータを保存する関数を呼び出す
            }
            break;

            case IDM_SAVE_LABELS_13:
            {
                std::wstring _msg = L"保存先に注意してください。\nスケールをかけてラベル出力します。\n1.13倍で保存します。";
                int _ID = MessageBox(hWnd, L"保存先に注意してください。\nスケールをかけてラベル出力します。", L"注意", MB_OKCANCEL);
                if (_ID == IDOK)
                {
                    SaveAnnotations(hWnd, L"書込ラベルフォルダを選択してください(スケール+13%)", 1.13f); // アノテーションデータを保存する関数を呼び出す
                }
            }
            break;
            case IDM_SAVE_LABELS_25:
            {
                std::wstring _msg = L"保存先に注意してください。\nスケールをかけてラベル出力します。\n1.25倍で保存します。";
                int _ID = MessageBox(hWnd, L"保存先に注意してください。\nスケールをかけてラベル出力します。", L"注意", MB_OKCANCEL);
                if (_ID == IDOK)
                {
                    SaveAnnotations(hWnd, L"書込ラベルフォルダを選択してください(スケール+25%)", 1.25f); // アノテーションデータを保存する関数を呼び出す
                }
            }
            break;
		    case IDM_SAVE_LABELS_50:
		    {
			    std::wstring _msg = L"保存先に注意してください。\nスケールをかけてラベル出力します。\n1.5倍で保存します。";
			    int _ID = MessageBox(hWnd, L"保存先に注意してください。\nスケールをかけてラベル出力します。", L"注意", MB_OKCANCEL);
			    if (_ID == IDOK)
			    {
				    SaveAnnotations(hWnd, L"書込ラベルフォルダを選択してください(スケール+50%)", 1.50f); // アノテーションデータを保存する関数を呼び出す
			    }
		    }
		    break;
		    case IDM_SAVE_LABELS_75:
		    {
			    std::wstring _msg = L"保存先に注意してください。\nスケールをかけてラベル出力します。\n1.75倍で保存します。";
			    int _ID = MessageBox(hWnd, L"保存先に注意してください。\nスケールをかけてラベル出力します。", L"注意", MB_OKCANCEL);
			    if (_ID == IDOK)
			    {
				    SaveAnnotations(hWnd, L"書込ラベルフォルダを選択してください(スケール+75%)", 1.75f); // アノテーションデータを保存する関数を呼び出す
			    }
		    }
		    break;
		    case IDM_SAVE_LABELS_100:
		    {
			    std::wstring _msg = L"保存先に注意してください。\nスケールをかけてラベル出力します。\n2.0倍で保存します。";
			    int _ID = MessageBox(hWnd, L"保存先に注意してください。\nスケールをかけてラベル出力します。", L"注意", MB_OKCANCEL);
			    if (_ID == IDOK)
			    {
				    SaveAnnotations(hWnd, L"書込ラベルフォルダを選択してください(スケール+100%)", 2.0f); // アノテーションデータを保存する関数を呼び出す
			    }
		    }
            break;
		    case IDM_NOSAVE_SMALL00: // 全て保存
		    {
			    GP.minimumLabelSize = 0.0f; // 最小サイズ制限を解除
                if(GP.minimumLabelSize == 0.0f)
                    MessageBox(hWnd, L"全てのオブジェクトを保存します。", L"注意", MB_OKCANCEL);
            }
		    break;
		    case IDM_NOSAVE_SMALL010: // 小さいラベルを保存しない
		    {
			    GP.minimumLabelSize = 0.01f; // 画面比1%以下の小さいオブジェクトを保存しない
			    if (GP.minimumLabelSize == 0.01f)
                    MessageBox(hWnd, L"保存先に注意してください。\n小さいオブジェクト(画面比1%以下)を消して保存します。", L"注意", MB_OKCANCEL);
		    }
		    break;
            case IDM_NOSAVE_SMALL015: // 小さいラベルを保存しない
            {
			    GP.minimumLabelSize = 0.015f; // 画面比1.5%以下の小さいオブジェクトを保存しない
			    if (GP.minimumLabelSize == 0.015f) 
                    MessageBox(hWnd, L"保存先に注意してください。\n小さいオブジェクト(画面比1.5%以下)を消して保存します。", L"注意", MB_OKCANCEL);
            }
		    break;
		    case IDM_NOSAVE_SMALL02: // 小さいラベルを保存しない
		    {
			    GP.minimumLabelSize = 0.02f; // 画面比2%以下の小さいオブジェクトを保存しない
                if (GP.minimumLabelSize == 0.02f)
                    MessageBox(hWnd, L"保存先に注意してください。\n小さいオブジェクト(画面比2%以下)を消して保存します。", L"注意", MB_OKCANCEL);
		    }
		    break;
		    case IDM_NOSAVE_SMALL03: // 小さいラベルを保存しない
		    {
			    GP.minimumLabelSize = 0.03f; // 画面比3%以下の小さいオブジェクトを保存しない
                if (GP.minimumLabelSize == 0.03f)
                    MessageBox(hWnd, L"保存先に注意してください。\n小さいオブジェクト(画面比3%以下)を消して保存します。", L"注意", MB_OKCANCEL);
		    }
		    break;
		    case IDM_NOSAVE_SMALL05: // 小さいラベルを保存しない
		    {
			    GP.minimumLabelSize = 0.05f; // 画面比5%以下の小さいオブジェクトを保存しない
                if (GP.minimumLabelSize == 0.05f)
                    MessageBox(hWnd, L"保存先に注意してください。\n小さいオブジェクト(画面比5%以下)を消して保存します。", L"注意", MB_OKCANCEL);
		    }
		    break;
		    case IDM_NOSAVE_SMALL10: // 小さいラベルを保存しない
		    {
			    GP.minimumLabelSize = 0.1f; // 画面比10%以下の小さいオブジェクトを保存しない
                if (GP.minimumLabelSize == 0.1f)
                    MessageBox(hWnd, L"保存先に注意してください。\n小さいオブジェクト(画面比10%以下)を消して保存します。", L"注意", MB_OKCANCEL);
		    }
            break;
            case IDM_NOSAVE_OR_CORRECT: 
            {
                if (GP.isMinimumLabelCrrect)
                    GP.isMinimumLabelCrrect = false;
                else
                    GP.isMinimumLabelCrrect = true;
            }
            break;
            case IDM_LOAD_IMAGES:
            {
                GP.imgFolderPath = GetFolderPathIFR(hWnd, L"読込イメージフォルダを選択してください"); // フォルダ選択ダイアログを表示

                if (!GP.imgFolderPath.empty()) {
				    // 画像ファイルのパスと矩形の配列をクリア
                    GP.imgObjs.clear();
                    // フォルダが選択された場合、画像ファイルを取得
                    LoadImageFilesMP(GP.imgFolderPath, GP.imgObjs); // フォルダ内の画像ファイルを取得
                    SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.imgIdx, (int)GP.imgObjs.size() ); // タイトルバーに画像のパスを表示

                    AutoDetctedObjs.objs.clear(); // AI推論結果をクリア

                    GP.imgIdx = 0; // インデックスをリセット
                }
                // 再描画
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;

            case IDM_LOAD_CLASSIFICATION:
		    {
			    //クラシフィケーションファイルを読込
			    std::wstring _filepath;
                // フィルタ設定
                COMDLG_FILTERSPEC filter[] ={
                    { L"クラシフィケーションファイル", L"*.txt" },
                    { L"プリアノテーションファイル",   L"*.onnx" },
                    { L"All Files",                    L"*.*"   }
                };
			    _filepath = GetFileName(hWnd, L"クラス分類ファイルを読込", filter, ARRAYSIZE(filter), 0);
                //_filepath = GetFileName(hWnd, L"クラス分類ファイルを読込", filter, 2, 0);
                if (!_filepath.empty()) {
				    // クラシフィケーションファイルを読込
				    LoadClassification(_filepath, GP.ClsNames, GP.ClsColors, GP.ClsDashStyles, GP.ClsPenWidths, 0);//0=読み込み、1=書き込み
			    }
		    }
		    break;

            case ID_ANNOT_JUMP_IGNOREBOX:
            {
                // startIdx は次の画像から検索したい場合は currentImageIdx+1
                auto result = jumpImgWithIgnoreBox(GP.imgObjs,
                    static_cast<size_t>(GP.imgIdx + 1),
					GDNNP.yolo.MINSIZEW / GDNNP.yolo.inputW,
                    GDNNP.yolo.MINSIZEH / GDNNP.yolo.inputH);

                if (result.has_value())
                {
                    // 前の画像に移動する前に、現在のラベルを保存
                    // 自動保存の場合は全て保存
                    SaveLabelsToFileSingle(hWnd, GP.imgIdx, 0.0f);
                    // 
                    GP.imgIdx = result.value() % GP.imgObjs.size();
                    SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.imgIdx, (int)GP.imgObjs.size()); // タイトルバーに画像のパスを表示

                    AutoDetctedObjs.objs.clear(); // AI推論結果をクリア
                    // 再描画
                    InvalidateRect(hWnd, nullptr, TRUE);
                }
                else
                {
                    MessageBox(hWnd,
                        L"不適切なアノテーションボックスは見つかりませんでした。",
                        L"ジャンプ",
                        MB_OK | MB_ICONINFORMATION);
                }
            }
            break;
			//現在の画像とラベルを削除 
			// 実際は削除はせず、画像は ..\delete\images\に、ラベルは ..\delete\labels\に移動する
            case IDM_DLETE_IMAGE:
            {
                // 現在の画像とラベルを移動
                if (GP.imgObjs.size() > 0 && GP.imgIdx < GP.imgObjs.size())
                {
                    // 指定idxの画像とラベルを移動
                    MoveCurrentImageAndLabel(hWnd, GP.imgIdx);
                    
                    // タイトルバーに画像のパスを表示
                    SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.imgIdx, (int)GP.imgObjs.size());

                    AutoDetctedObjs.objs.clear(); // AI推論結果をクリア
                    // 再描画
                    InvalidateRect(hWnd, nullptr, TRUE);
                }
			}
            break;
            case IDM_JUNP_NO_ANNOTATION:
            {
                // アノテーションのない画像にジャンプ
                JumpToUnlabeledImage(hWnd);
			}
            break;

			case IDN_FIX_LABELS_IMAGE: // 異常なラベルを修正(小さすぎるラベルを修正)
            {
                //警告 画像解像度の確認
                std::wstring _inputW = std::to_wstring(GDNNP.yolo.inputW);
                std::wstring _inputH = std::to_wstring(GDNNP.yolo.inputH);
                std::wostringstream _msg;
                _msg << L"全ての画像のラベルを修正します。\n"
                    << L"画像解像度が異なる場合、意図しない修正になる場合があります。\n"
                    << L"現在の設定解像度: "
                    << _inputW + L" x " << _inputH << L"\n"
                    << L"よろしいですか？";

                int ret = MessageBoxW(hWnd, _msg.str().c_str(),
                    L"確認", MB_OKCANCEL | MB_ICONWARNING);
                if (ret != IDCANCEL)
                {
                    //現在の画像のラベルを画像サイズに合わせて修正
                    float minW = GDNNP.yolo.MINSIZEW / GDNNP.yolo.inputW;
                    float minH = GDNNP.yolo.MINSIZEH / GDNNP.yolo.inputH;
                    int ret = FixLabelBox_in_ImgObj(GP.imgObjs[GP.imgIdx], minW, minH);

                    if (ret > 0) {
                        // 再描画
                        InvalidateRect(hWnd, NULL, TRUE);
                        std::wstring msg = L"異常なラベルを修正しました。\n修正したラベル数: " + std::to_wstring(ret);
                        MessageBox(hWnd, msg.c_str(), L"情報", MB_OK | MB_ICONINFORMATION);
                    }
                    else {
                        MessageBox(hWnd, L"異常なラベルはありません。", L"情報", MB_OK | MB_ICONINFORMATION);
                    }
                }
			}
            break;

            case IDN_FIX_LABELS_ALLIMAGE: // 異常なラベルを修正(小さすぎるラベルを修正)
			{  
                //警告 画像解像度の確認
				std::wstring _inputW = std::to_wstring(GDNNP.yolo.inputW);
                std::wstring _inputH = std::to_wstring(GDNNP.yolo.inputH);
                std::wostringstream _msg;
                _msg << L"全ての画像のラベルを修正します。\n"
                     <<L"画像解像度が異なる場合、意図しない修正になる場合があります。\n"
                     <<L"現在の設定解像度: "
                     << _inputW + L" x " << _inputH << L"\n"
                     << L"よろしいですか？";

                int ret = MessageBoxW(hWnd, _msg.str().c_str(),
                    L"確認", MB_OKCANCEL | MB_ICONWARNING);
				if (ret != IDCANCEL)
                {
                    //全ての画像のラベルを画像サイズに合わせて修正
                    float minW = GDNNP.yolo.MINSIZEW / GDNNP.yolo.inputW;
                    float minH = GDNNP.yolo.MINSIZEH / GDNNP.yolo.inputH;
                    int total = 0;
                    for (size_t i = 0; i < GP.imgObjs.size(); i++) {
                        int ret = FixLabelBox_in_ImgObj(GP.imgObjs[i], minW, minH);
                        total += ret;
                    }
                    if (total > 0) {
                        // 再描画
                        InvalidateRect(hWnd, NULL, TRUE);
                        std::wstring msg = L"異常なラベルを修正しました。\n修正したラベル数: " + std::to_wstring(total);
                        MessageBox(hWnd, msg.c_str(), L"情報", MB_OK | MB_ICONINFORMATION);

                        //ラベル保存の処理をPostする
                        PostMessage(hWnd, WM_COMMAND, IDM_SAVE_LABELS, 0);
                    }
                    else {
                        MessageBox(hWnd, L"異常なラベルはありません。", L"情報", MB_OK | MB_ICONINFORMATION);
                    }
                }
			}
			break;

            // 現在のイメージのラベリングされたオブジェクトの一覧のポップアップメニューを作成する
            case IDM_PMENU_LABEL_BASE:
			    CreatePopupMenuFor_Labels_in_CurrentImage(hWnd);
                break;

            case IDM_YOLO_PRESETBOX:
            {
                AutoDetctedObjs.objs.clear(); // AI推論結果をクリア

                cv::Mat _img = cv::imread(WStringToString(GP.imgObjs[GP.imgIdx].path)); // 画像をOpenCVで読み込み
                //DnnParams p;
                //p.opt.backend = cv::dnn::DNN_BACKEND_OPENCV; // or DNN_BACKEND_CUDA
                //p.opt.target = cv::dnn::DNN_TARGET_CPU;     // or DNN_TARGET_CUDA
                //p.yolo.inputW = 640; p.yolo.inputH = 640;
                //p.yolo.confThreshold = 0.25f;
                //p.yolo.nmsThreshold = 0.45f;
                //AutoDetctedObjs.objs = DnnInfer(_img, L".\\yolov5s.onnx",p);
                //AutoDetctedObjs.objs = DnnInfer(_img, g_onnxFile, p);

                AutoDetctedObjs.objs = DnnInfer(GDNNP.yolo.YoloVersion, _img, GDNNP.gOnnxPath, GDNNP);

                g_showProposals = true; //これ要るの?

                // 再描画
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;

            case IDM_YOLO_PRESETPARAM:
            {
                RECT rcClient;
                GetClientRect(hWnd, &rcClient);
                SliderDialogT<float>* pDlgf = nullptr;

                pDlgf = new SliderDialogT<float>(
                    hInst,
                    hWnd,
                    GDNNP.yolo.nmsThreshold,
                    0.0f, 1.0f,                 // min, max
                    true,                       // 横型
                    L"NMS",                     // ラベル文字列
                    rcClient.right - 60, 0,     // ← 初期位置 x, y 
                    60, 250                     // ← 初期サイズ width, height
                );
                pDlgf->ShowModeless(hWnd);      // ← 即リターンしてメイン処理継続

                pDlgf = new SliderDialogT<float>(
                    hInst,
                    hWnd,
                    GDNNP.yolo.confThreshold,
                    0.0f, 1.0f,                 // min, max
                    true,                       // 横型
                    L"Conf",                    // ラベル文字列
                    rcClient.right - 60 - 60, 0,   // ← 初期位置 x, y 
                    60, 250                     // ← 初期サイズ width, height
                );
                pDlgf->ShowModeless(hWnd);  // ← 即リターンしてメイン処理継続

            }
            break;

            case IDM_YOLO_MAXSENCE1:
            {
				GDNNP.yolo.confThreshold = 0.01f;
                CheckMenues(hWnd);
            }
            break;
            case IDM_YOLO_MAXSENCE2:
            {
                GDNNP.yolo.confThreshold = 0.005f;
                CheckMenues(hWnd);
            }
            break;
            case IDM_YOLO_MAXSENCE3:
            {
                GDNNP.yolo.confThreshold = 0.001f;
                CheckMenues(hWnd);
            }
            break;

            case IDM_YOLO_MAXSENCE4:
            {
                GDNNP.yolo.confThreshold = 0.0f;
                CheckMenues(hWnd);
            }
            break;

            case IDM_YOLO_IMGSZE640:
            {
                GDNNP.yolo.inputW = 640;
                GDNNP.yolo.inputH = 640;
                CheckMenues(hWnd);
            }
            break;
            case IDM_YOLO_IMGSZE1280:
            {
                GDNNP.yolo.inputW = 1280;
                GDNNP.yolo.inputH = 1280;
                CheckMenues(hWnd);
            }
            break;
            case IDM_YOLO_IMGSZE1920:
            {
                GDNNP.yolo.inputW = 1920;
                GDNNP.yolo.inputH = 1920;
                CheckMenues(hWnd);
            }
            break;
            case IDM_YOLO_SELCTONNX:
            {
                // DNN ONNXモデルファイルを選択
                std::wstring _filepath;
                //_filepath = GetFileName(hWnd, L"DNN ONNXモデルファイルを選択", 0);
				//_filepath = _ChangeFileExtension( L"DNN ONNXモデルファイルを選択", L"*.onnx"); // フォルダ選択ダイアログを表示

                COMDLG_FILTERSPEC filter[] = {
                    { L"プリアノテーションファイル",   L"*.onnx" },
                    { L"クラシフィケーションファイル", L"*.txt" },
                    { L"All Files",                    L"*.*"   }
                };
                _filepath = GetFileName(hWnd, L"DNN ONNXモデルファイルを選択", filter, ARRAYSIZE(filter), 0);

                if (!_filepath.empty()) {
                    GDNNP.gOnnxPath = _filepath; // ファイルパスを保存
                }
				CheckMenues(hWnd);
            }break;

            case IDM_YOLO_V5_V7:
            {
                GDNNP.yolo.YoloVersion = 5;
                CheckMenues(hWnd);
            }break;

            case IDM_YOLO_V8_V11:
            {
                GDNNP.yolo.YoloVersion = 8;
				CheckMenues(hWnd);
			}break;

            case IDM_ONNX000 + 1:  { set_onnx_files_in_menu(0);  CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 2:  { set_onnx_files_in_menu(1);  CheckMenues(hWnd);}  break;
			case IDM_ONNX000 + 3:  { set_onnx_files_in_menu(2);  CheckMenues(hWnd);}  break;
			case IDM_ONNX000 + 4:  { set_onnx_files_in_menu(3);  CheckMenues(hWnd);}  break;
			case IDM_ONNX000 + 5:  { set_onnx_files_in_menu(4);  CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 6:  { set_onnx_files_in_menu(5);  CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 7:  { set_onnx_files_in_menu(6);  CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 8:  { set_onnx_files_in_menu(7);  CheckMenues(hWnd);}  break;
			case IDM_ONNX000 + 9:  { set_onnx_files_in_menu(8);  CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 10: { set_onnx_files_in_menu(9);  CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 11: { set_onnx_files_in_menu(10); CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 12: { set_onnx_files_in_menu(11); CheckMenues(hWnd);}  break;
			case IDM_ONNX000 + 13: { set_onnx_files_in_menu(12); CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 14: { set_onnx_files_in_menu(13); CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 15: { set_onnx_files_in_menu(14); CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 16: { set_onnx_files_in_menu(15); CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 17: { set_onnx_files_in_menu(16); CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 18: { set_onnx_files_in_menu(17); CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 19: { set_onnx_files_in_menu(18); CheckMenues(hWnd);}  break;
            case IDM_ONNX000 + 20: { set_onnx_files_in_menu(19); CheckMenues(hWnd);}  break;

            case IDM_EDIT_CLEAR_LABELS:
            {
                //ClearCurrentImageLabels(hWnd);
                if (GP.imgObjs.empty()) 
                    break;

                //Undo処理のために、現在のラベルを保存
                GP.undo.lastCleared = GP.imgObjs[GP.imgIdx].objs;
                GP.undo.hasUndo = true;
                GP.undo.imgIdxUndo = GP.imgIdx;

                auto& cur = GP.imgObjs[GP.imgIdx];

                if (!cur.objs.empty()) {
                    cur.objs.clear();          // ラベルを全消去
                    cur.isEdited = true;       // 編集フラグON（後で保存判定に使えます）:contentReference[oaicite:0]{index=0}
                    InvalidateRect(hWnd, nullptr, TRUE);  // 再描画
                    // 必要なら通知音：
                    // PlaySound(SOUND_ARRY[37][1], NULL, SND_ALIAS | SND_ASYNC | SND_NODEFAULT);
                }
                AutoDetctedObjs.objs.clear();
                AutoDetctedObjs.objIdx = 0;
                g_showProposals = false;

                PlaySound(SOUND_ARRY[37][1], NULL, SND_ALIAS | SND_ASYNC | SND_NODEFAULT);

            }break;

            case IDM_UNDO_CLEAR_LABELS:
            {
                if (GP.imgObjs.empty())
                    break;
                UndoClear();

                InvalidateRect(hWnd, nullptr, TRUE);  // 再描画
                // 必要なら通知音：
				//PlaySound(SOUND_ARRY[37][1], NULL, SND_ALIAS | SND_ASYNC | SND_NODEFAULT);
            }break;

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

            default:
            {
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
    }
    break;
    //メニューここまで
    //////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////
    // 描画処理
    //////////////////////////////////////////////////////////////////////////////////
    case WM_PAINT:
    {
        if(!GP.isCompare)
    		DoPaint(hWnd, wParam, lParam, GP.imgIdx); // 描画処理を行う関数を呼び出す
        else
            DoPaint(hWnd, wParam, lParam, GP.imgIdxCompare); // 描画処理を行う関数を呼び出す

    }
    break;

    //バッグラウンド描画を無効にする
    case WM_ERASEBKGND: 
        return 1;  // 背景の塗りつぶし処理を行った（何もしないけど）ことをOSに伝える

    case WM_SIZE:
    {
        // ウィンドウのサイズ変更時の処理
        //RECT rect;
        GetClientRect(hWnd, &GP.rect_win);
        GP.width = GP.rect_win.right - GP.rect_win.left;
        GP.height = GP.rect_win.bottom - GP.rect_win.top;
        // 画像をウィンドウのサイズに合わせて描画する場合は、ここで処理を追加
    }
    break;

    ////////////////////////////////////////////////////
    // マウスの左ボタンでダブルクリックしたときの処理
    ////////////////////////////////////////////////////
    case WM_LBUTTONDBLCLK:
    {
		//バグりそうなので無効化
    }
    break;

	// マウスの左ボタンが押されたときの処理
	case WM_LBUTTONDOWN:
	{
        if(GP.imgObjs.size() > 0)
        {
            // ビューポートを算出
            RECT rc; GetClientRect(hWnd, &rc);
            //auto bmp = GP.imgObjs[GP.imgIdx].image;
            UINT imgW = GP.imgObjs[GP.imgIdx].image.get()->GetWidth();
            UINT imgH = GP.imgObjs[GP.imgIdx].image.get()->GetHeight();
            auto vp = ComputeViewport(rc, imgW, imgH);

            // マウスの位置を取得しビューポート座標に変換
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);

            // ビューポート座標に変換
            float vp_x = float(pt.x - vp.dest.X) / float(vp.dest.Width);
            float vp_y = float(pt.y - vp.dest.Y) / float(vp.dest.Height);

            // マウスの位置を取得
            //POINT pt;
            //GetCursorPos(&pt);
            //ScreenToClient(hWnd, &pt);

            //矩形にマウスオーバーしていない時は矩形を作成する。
            if (GP.imgObjs[GP.imgIdx].mOverIdx == -1)
            {
                // 矩形の開始位置を設定
                GP.tmpLabel.Rct.X = vp_x;
                GP.tmpLabel.Rct.Y = vp_y;
                GP.tmpLabel.Rct.Width = 0;
                GP.tmpLabel.Rct.Height = 0;
                GP.dgMode = DragMode::MakeBox;
            }
            //矩形にマウスオーバーしている
            else
            {
                // マウス位置を保存
                GP.prevMouse = Gdiplus::PointF((FLOAT)pt.x, (FLOAT)pt.y);
                // ヒット中の矩形のインデックスを保存
                GP.activeIdx = GP.imgObjs[GP.imgIdx].mOverIdx;
                GP.dgMode = DragMode::ReBox; // ドラッグ中フラグを立てる
            }
            SetCapture(hWnd); // マウスキャプチャを取得 ウィンドウの外に出てもマウスイベントを受け取る
        }
    }
	break;

	////////////////////////////////////////////////////
    // 
	// マウスの左ボタンが離されたときの処理
    // 
	////////////////////////////////////////////////////
	case WM_LBUTTONUP:
	{
		//if (GP.makeBox)
        if (GP.dgMode == DragMode::MakeBox)
        {
            // ビューポートを算出
            RECT rc; GetClientRect(hWnd, &rc);
            //auto bmp = GP.imgObjs[GP.imgIdx].image;
            UINT imgW = GP.imgObjs[GP.imgIdx].image.get()->GetWidth();
            UINT imgH = GP.imgObjs[GP.imgIdx].image.get()->GetHeight();
            auto vp = ComputeViewport(rc, imgW, imgH);

            // マウスの位置を取得しビューポート座標に変換
            //POINT pt_org;
            //GetCursorPos(&pt_org);
            //ScreenToClient(hWnd, &pt_org);
            //POINT pt{ pt_org.x - (LONG)vp.origin.X, pt_org.y - (LONG)vp.origin.Y };
            // マウスの位置を取得

            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);
            
            // ビューポート座標に変換
            float vp_x = float(pt.x - vp.dest.X) / float(vp.dest.Width);
            float vp_y = float(pt.y - vp.dest.Y) / float(vp.dest.Height);

            ReleaseCapture(); // マウスキャプチャを解放

            // 矩形の幅と高さを計算
            GP.tmpLabel.Rct.Width = vp_x - GP.tmpLabel.Rct.X;
            GP.tmpLabel.Rct.Height = vp_y - GP.tmpLabel.Rct.Y;
            NormalizeRect(GP.tmpLabel.Rct); // 矩形の座標を正規化

			//クラス名をポップアップメニューで表示
            ShowClassPopupMenu(hWnd, false);

            ReleaseCapture();
        }
        else if (GP.dgMode == DragMode::ReBox) // 矩形の編集モード
        {
            // マウスキャプチャを解放
            ReleaseCapture();
        }
        else
        {
			ReleaseCapture();
        }
        // 再描画
		InvalidateRect(hWnd, NULL, TRUE);

        // 後始末
        GP.dgMode = DragMode::None; // ドラッグ中フラグを下ろす
        GP.edMode = EditMode::None; // 編集モードをリセット

        // テンポラリ矩形の座標をリセット
        GP.tmpLabel.Rct.X = 0;
        GP.tmpLabel.Rct.Y = 0;
        GP.tmpLabel.Rct.Width = 0;
        GP.tmpLabel.Rct.Height = 0;
        //GP.makeBox = false; // ドラッグ中フラグを下ろす
        GP.dgMode = DragMode::None;
	}
	break;
    

    ////////////////////////////////////////////////////
	// マウスの右ボタンが押されたときの処理
    ////////////////////////////////////////////////////
	case WM_RBUTTONDOWN:
	{
        if(GP.imgObjs.size()>0)
        {
            // ビューポートを算出
            RECT rc; GetClientRect(hWnd, &rc);
            //auto bmp = GP.imgObjs[GP.imgIdx].image;
            UINT imgW = GP.imgObjs[GP.imgIdx].image.get()->GetWidth();
            UINT imgH = GP.imgObjs[GP.imgIdx].image.get()->GetHeight();
            auto vp = ComputeViewport(rc, imgW, imgH);

            // マウスの位置を取得しビューポート座標に変換
            POINT pt_org;
            GetCursorPos(&pt_org);
            ScreenToClient(hWnd, &pt_org);
            POINT pt{ pt_org.x - (LONG)vp.origin.X, pt_org.y - (LONG)vp.origin.Y };

            // マウスの位置を取得
            //POINT pt;
            //GetCursorPos(&pt);
            //ScreenToClient(hWnd, &pt);


            // ヒット中の矩形のインデックスを保存
            GP.activeIdx = GP.imgObjs[GP.imgIdx].mOverIdx;
            // YOLOの推論結果のバウンディングボックスの場合
            int AutoDetctedObjsIdx = AutoDetctedObjs.mOverIdx;

            // AI推定で無い方を優先 両方ヒットする可能性もあるので
            if (GP.activeIdx != -1)
            {
                // クラス名をポップアップメニューで表示
                ShowClassPopupMenu_for_Edit(hWnd, GP.activeIdx, false);
            }
            //自動アノテーションのバウンディングボックスの場合
            else if (AutoDetctedObjsIdx != -1)
            {
                // クラス名をポップアップメニューで表示
                int _ret = ShowClassPopupMenu_for_Edit(hWnd, AutoDetctedObjs, AutoDetctedObjsIdx, true);
                if (_ret >= 0)
                {
                    // 選択された場合、メインの配列に矩形を追加する
                    GP.imgObjs[GP.imgIdx].isEdited = true; // 編集されたことにする
                    GP.imgObjs[GP.imgIdx].objs.push_back(AutoDetctedObjs.objs[AutoDetctedObjsIdx]);
                    AutoDetctedObjs.objs.erase(AutoDetctedObjs.objs.begin() + AutoDetctedObjsIdx);
                }
                else if (_ret == -10) //そのまま確定
                {
                    //★上と同じ処理
                    // 選択された場合、メインの配列に矩形を追加する
                    GP.imgObjs[GP.imgIdx].isEdited = true; // 編集されたことにする
                    GP.imgObjs[GP.imgIdx].objs.push_back(AutoDetctedObjs.objs[AutoDetctedObjsIdx]);
                    AutoDetctedObjs.objs.erase(AutoDetctedObjs.objs.begin() + AutoDetctedObjsIdx);
                }

            }
        }
    }
    break;


    case WM_MBUTTONDOWN:
    {
        SetCapture(hWnd);
        POINT pt; GetCursorPos(&pt); ScreenToClient(hWnd, &pt);
        GP.panLast = pt;
		GP.dgMode = DragMode::Pan; // ドラッグ中フラグを立てる
        // 視覚フィードバックが欲しければ:
        // SetCursor(LoadCursor(NULL, IDC_SIZEALL));
    }
    break;

    case WM_MBUTTONUP:
    {
        if (GP.dgMode == DragMode::Pan) {
            ReleaseCapture();
			GP.dgMode = DragMode::None; // ドラッグ中フラグを下ろす
            //GP.isPanning = false;
            // SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
    }
    break;


    // マウスの移動中の処理
	// BOXと重なっているかどうかを判定したり、矩形の編集を行う
    case WM_MOUSEMOVE:
    {
        if (GP.imgObjs.size() > 0) // 画像が存在する場合
        {
            // ビューポートを算出
            RECT rc;
            GetClientRect(hWnd, &rc);
            //auto bmp = GP.imgObjs[GP.imgIdx].image;
            UINT imgW = GP.imgObjs[GP.imgIdx].image.get()->GetWidth();
            UINT imgH = GP.imgObjs[GP.imgIdx].image.get()->GetHeight();
            auto vp = ComputeViewport(rc, imgW, imgH);


            // マウスの位置を取得しビューポート座標に変換
            //POINT pt_org;
            //GetCursorPos(&pt_org);
            //ScreenToClient(hWnd, &pt_org);
            //POINT pt{ pt_org.x - (LONG)vp.origin.X, pt_org.y - (LONG)vp.origin.Y };

            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);

            // ビューポート座標に変換
            float vp_x = float(pt.x - vp.dest.X) / float(vp.dest.Width);
            float vp_y = float(pt.y - vp.dest.Y) / float(vp.dest.Height);

            //if (GP.makeBox)
            if (GP.dgMode == DragMode::MakeBox)
            {
                GP.tmpLabel.Rct.Width = vp_x - GP.tmpLabel.Rct.X;
                GP.tmpLabel.Rct.Height = vp_y - GP.tmpLabel.Rct.Y;

                // 再描画
                InvalidateRect(hWnd, NULL, TRUE);
            }
            else if (GP.dgMode == DragMode::ReBox) // 矩形の編集モード
            {
                // マウス移動分を計算
                //prevMouse はフロートだが画面のピクセルサイズのスケール
                float dx = float(pt.x) - GP.prevMouse.X;
                float dy = float(pt.y) - GP.prevMouse.Y;

                if (!GP.imgObjs[GP.imgIdx].objs.empty())//objsが空でない場合 
                {
                    // 矩形の編集モードで、マウス移動分を適用
                    auto& r = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct;

                    GP.imgObjs[GP.imgIdx].isEdited = true; // 編集されたことにする
                    switch (GP.edMode)
                    {
                    case EditMode::Top:
                        // 矩形サイズが小さく、かつ、小さい方向に動かす場合は、上辺を動かさない 
                        // 上辺を dy だけ動かす → 矩形の高さは −dy だけ変わる
                        r.Y += dy / vp.dest.Height;
                        r.Height -= dy / vp.dest.Height;
                        break;
                    case EditMode::Bottom:
                        // 矩形サイズが小さく、かつ、小さい方向に動かす場合は、上辺を動かさない 
                        //if (isSmallRectY && dy < 0)
                        //    break; // 編集しない

                        // 下辺を dy だけ動かす → 矩形の高さは +dy だけ変わる
                        r.Height += dy / vp.dest.Height;
                        break;
                    case EditMode::Left:
                        // 矩形サイズが小さく、かつ、小さい方向に動かす場合は、左辺を動かさない 
                        //if (isSmallRectX && dx > 0)
                        //    break; // 編集しない

                        // 左辺を dx だけ動かす → 矩形の幅は −dx だけ変わる
                        r.X += dx / vp.dest.Width;
                        r.Width -= dx / vp.dest.Width;
                        break;
                    case EditMode::Right:
                        // 矩形サイズが小さく、かつ、小さい方向に動かす場合は、左辺を動かさない 
                        //if (isSmallRectX && dx < 0)
                        //    break; // 編集しない

                        // 右辺を dx だけ動かす → 矩形の幅は +dx だけ変わる
                        r.Width += dx / vp.dest.Width;
                        break;
                    case EditMode::LeftTop:
                        // 左上を dx,dy だけ動かす → 矩形の幅は −dx だけ変わる
                        // 矩形の高さは −dy だけ変わる
                        r.X += dx / vp.dest.Width;
                        r.Y += dy / vp.dest.Height;
                        r.Width -= dx / vp.dest.Width;
                        r.Height -= dy / vp.dest.Height;
                        break;
                    case EditMode::RightTop:
                        // 右上を dx,dy だけ動かす → 矩形の幅は +dx だけ変わる
                        // 矩形の高さは −dy だけ変わる
                        r.Y += dy / vp.dest.Height;
                        r.Width += dx / vp.dest.Width;
                        r.Height -= dy / vp.dest.Height;
                        break;
                    case EditMode::LeftBottom:
                        // 左下を dx,dy だけ動かす → 矩形の幅は −dx だけ変わる
                        // 矩形の高さは +dy だけ変わる
                        r.X += dx / vp.dest.Width;
                        r.Height += dy / vp.dest.Height;
                        r.Width -= dx / vp.dest.Width;
                        break;
                    case EditMode::RightBottom:
                        // 右下を dx,dy だけ動かす → 矩形の幅は +dx だけ変わる
                        // 矩形の高さは +dy だけ変わる
                        r.Width += dx / vp.dest.Width;
                        r.Height += dy / vp.dest.Height;
                        break;
                    default:
                        break;
                    }
                    // マウス位置を保存
                    //if(!isSmallRectX&& !isSmallRectY)
                    GP.prevMouse = Gdiplus::PointF((FLOAT)pt.x, (FLOAT)pt.y);

                    // 再描画
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
            else if (GP.dgMode == DragMode::dummy) // 矩形の編集モード
            {
                if (GP.imgIdx < GP.imgObjs.size()) {              // ← これが先
                    auto& curImg = GP.imgObjs[GP.imgIdx];         // 安全に参照取得
                    if (!curImg.objs.empty()) {
                        curImg.mOverIdx =
                            GetIdxMouseOnRectEdgeVP(pt, curImg.objs,
                                GP.edMode, GP.Overlap, vp.dest);
                    }
                }

                switch (GP.edMode)
                {
                case EditMode::Top:
                    // 上辺をドラッグ。 直接、マウスの値を辺の位置に設定
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y = vp_y;
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Height = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y - GP.prevMouse.Y;
                    break;
                case EditMode::Bottom:
                    // 下辺をドラッグ。 直接、マウスの値を辺の位置に設定
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Height = vp_y - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y;
                    break;
                case EditMode::Left:
                    // 左辺をドラッグ。 直接、マウスの値を辺の位置に設定
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X = vp_x;
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Width = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X - GP.prevMouse.X;
                    break;
                case EditMode::Right:
                    // 右辺をドラッグ。 直接、マウスの値を辺の位置に設定
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Width = vp_x - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X;
                    break;
                case EditMode::LeftTop:
                    // 左上をドラッグ。 直接、マウスの値を辺の位置に設定   
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X = vp_x;
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y = vp_y;
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Width = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X - GP.prevMouse.X;
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Height = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y - GP.prevMouse.Y;
                    break;
                case EditMode::RightTop:
                    // 右上をドラッグ。 直接、マウスの値を辺の位置に設定
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y = vp_y;
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Width = vp_x - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X;
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Height = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y - GP.prevMouse.Y;
                    break;
                case EditMode::LeftBottom:
                    // 左下をドラッグ。 直接、マウスの値を辺の位置に設定
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X = vp_x;
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Height = vp_y - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y;
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Width = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X - GP.prevMouse.X;
                    break;
                case EditMode::RightBottom:
                    // 右下をドラッグ。 直接、マウスの値を辺の位置に設定
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Width = vp_x - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X;
                    GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Height = vp_y - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y;
                    break;
                case EditMode::None:
                    // 何もしない    
                    break;
                }
                // マウスの位置を保存
                GP.prevMouse = Gdiplus::PointF((FLOAT)pt.x, (FLOAT)pt.y);
                // 再描画
                InvalidateRect(hWnd, NULL, TRUE);
            }
            else if (GP.dgMode == DragMode::Pan) // パンモード
            {
                // マウス移動分を計算
                POINT pt; GetCursorPos(&pt); ScreenToClient(hWnd, &pt);

                int dx = pt.x - GP.panLast.x;
                int dy = pt.y - GP.panLast.y;

                // ピクセル単位で viewOffset に加算
                GP.viewOffset.X += static_cast<float>(dx);
                GP.viewOffset.Y += static_cast<float>(dy);

                GP.panLast = pt;
                InvalidateRect(hWnd, nullptr, TRUE);
                //break; // パン中は他の編集ロジックはスキップ
            }

            else // BOXと重なっているかどうかを判定
            {
                // マウスカーソルと矩形が重なっていないかどうかを調べて、
                // 重なっていたらそのインデックスを取得し、現在アクティブな画像のmOverIdxに格納
                if (GP.imgIdx < GP.imgObjs.size()) {              // ← これが先
                    auto& curImg = GP.imgObjs[GP.imgIdx];         // 安全に参照取得
                    if (!curImg.objs.empty())
                    {
                        const int overlap = GP.Overlap; // 既存設定でOK

                        curImg.mOverIdx = GetIdxMouseOnRectEdgeVP(pt,
                            GP.imgObjs[GP.imgIdx].objs,
                            GP.edMode,
                            overlap,
                            vp.dest);

                        //if (hit != -1)
                        //{
                        //    GP.imgObjs[GP.imgIdx].objIdx = (int)hit;
                        //}
                        //curImg.mOverIdx =
                        //    GetIdxMouseOnRectEdge(pt, curImg.objs,
                        //        GP.edMode, GP.Overlap);
                    }
                }
                //上と同じ安全措置必要か
                EditMode _dummy; //edModeは矩形の辺の位置を調べるのに必要だが、ここでは使わないのでダミー変数
                //AutoDetctedObjs.mOverIdx = GetIdxMouseOnRectEdge(pt, AutoDetctedObjs.objs, GP.edMode, GP.Overlap); // マウスカーソルの位置を取得
                //AutoDetctedObjs.mOverIdx = GetIdxMouseOnRectEdge(pt, AutoDetctedObjs.objs, _dummy, GP.Overlap); // マウスカーソルの位置を取得
                AutoDetctedObjs.mOverIdx = GetIdxMouseOnRectEdgeVP(pt, AutoDetctedObjs.objs, _dummy, GP.Overlap, vp.dest); // マウスカーソルの位置を取得

                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
    }
    break;

    case WM_MOUSEWHEEL:
    {
        if (GP.imgObjs.empty() || !GP.imgObjs[GP.imgIdx].image) break;

        POINT ptScr{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hWnd, &ptScr);

        const auto& bmp = GP.imgObjs[GP.imgIdx].image;
        UINT imgW = bmp->GetWidth(), imgH = bmp->GetHeight();

        // 現在のビューポート
        RECT rc; GetClientRect(hWnd, &rc);
        auto vpBefore = ComputeViewport(rc, imgW, imgH);

        // スクロール方向で倍率
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        float step = (delta > 0) ? 1.1f : 1.0f / 1.1f;

        // 不動点（カーソル下の画像上の正規化座標）を求める
        Gdiplus::PointF fixed01 = ScreenPtToNorm((float)ptScr.x, (float)ptScr.y, vpBefore, imgW, imgH);

        // 新しいズームに更新（クランプ）
        GP.zoom = std::clamp(GP.zoom * step, GP.minZoom, GP.maxZoom);

        // 新ビューポート
        auto vpAfter = ComputeViewport(rc, imgW, imgH);

        if (GP.zoomToCursor) {
            // 同じ画像上の点が同じスクリーン位置に来るようオフセットを補正
            // 目標スクリーン座標 = 不動点(画像)を新ビューポートへ投影
            float targetX = fixed01.X * imgW * vpAfter.scale + vpAfter.origin.X;
            float targetY = fixed01.Y * imgH * vpAfter.scale + vpAfter.origin.Y;
            // 目標が現在のマウス位置に一致するようoriginを動かす ⇒ viewOffset を補正
            GP.viewOffset.X += (ptScr.x - targetX);
            GP.viewOffset.Y += (ptScr.y - targetY);
        }
        InvalidateRect(hWnd, nullptr, TRUE);
    }
    break;

	// キー入力を処理する 
    // Altキーの処理はWM_SYSKEYDOWNで処理する
    case WM_KEYDOWN:
    {   // swtch文で入力されたキーを判定、処理を分ける
        // swtch文が二重になっている(Windowsプログラミングでは定石)

		// 移動する量を計算
        // Shift が押されているかどうかを調べる
        bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        int step = 1;
		// Ctrl が押されているかどうかを調べる
		bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        bool alt = (GetKeyState(VK_MENU) & 0x8000) != 0; // Altキーが押されているかどうかを調べる

		// 何らかのキーダウンがあれば、比較モードをオフにする
        GP.isCompare = false; // 比較モードをオフ

        if (shift){   // Shift＋矢印なら大きく移動、小なら通常移動
            step = shift ? 10 : step;
        }
		else if (ctrl) {   // Ctrl＋矢印なら大きく移動
            step = ctrl ? 100 : step;
        }
        else{              // 通常の移動量
			step = 1; 
        }

        switch (wParam)
        {
            case 'A': // 前の画像
            case VK_LEFT:
            if (GP.imgObjs.size() > 0) {

				// 前の画像に移動する前に、現在のラベルを保存
                // 自動保存の場合は全て保存
                SaveLabelsToFileSingle(hWnd, GP.imgIdx, 0.0f);
                // 次の画像に移動 stepの値分だけ移動する
                GP.imgIdx = (GP.imgIdx + GP.imgObjs.size() - step) % GP.imgObjs.size();
                SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.imgIdx, (int)GP.imgObjs.size()); // タイトルバーに画像のパスを表示

                AutoDetctedObjs.objs.clear(); // AI推論結果をクリア
            }
            break;

		    case 'D': // 次の画像 alt+ctrlならラベル定義のない画像まで移動
            case VK_RIGHT:
            {
                if (GP.imgObjs.size() > 0) {
                    if (alt && ctrl) { // Altキーが押されている場合
                        if (GP.imgIdx < (GP.imgObjs.size() - 1)) {
                            step = 1;
                            while (!GP.imgObjs[GP.imgIdx + step].objs.empty()) {
                                step++;
                                if (GP.imgIdx + step >= GP.imgObjs.size()) { // 画像の範囲を超えたら

                                    step = 0; // 0に戻す
                                    break; // ループを抜ける
                                }
                            }
                        }
                    }
                    // 前の画像に移動する前に、現在のラベルを保存
                    // 自動保存の場合は全て保存
                    SaveLabelsToFileSingle(hWnd, GP.imgIdx, 0.0f);
                    // 次の画像に移動 stepの値分だけ移動する
                    GP.imgIdx = (GP.imgIdx + step) % GP.imgObjs.size();
                    SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.imgIdx, (int)GP.imgObjs.size()); // タイトルバーに画像のパスを表示

                    AutoDetctedObjs.objs.clear(); // AI推論結果をクリア
                }
            }
            break;

            case 'v': //前の画像からコピー
            case 'V': //前の画像からコピー
            case 'c': //前の画像からコピー
            case 'C': //前の画像からコピー
            {    //if (ctrl){
                if (GP.imgIdx > 0) {
                    //コピー前に確認のメッセージを表示
                    int _copyok = MessageBoxW(hWnd, L"前の画像からコピーしますか？", L"確認", MB_OKCANCEL);

                    if (_copyok == IDOK) {
                        // コピー処理
                        GP.imgObjs[GP.imgIdx].objs = GP.imgObjs[GP.imgIdx - 1].objs;
                        InvalidateRect(hWnd, NULL, TRUE); // 再描画
                    }
                }
                //}
            }
            break;

            case 'S': //前後画像の比較
            case 's':
            {
                if (GP.isCompare)
                    GP.isCompare = false; // 比較モードをオフ
                else
                {
                    if (GP.imgIdx == GP.imgObjs.size() - 1) // 最後の画像でない場合
                        break; // 最後の画像の場合は何もしない
                    else
                    {
                        GP.isCompare = true; // 比較モードをオン
                        SetTimer(hWnd, IDT_COMPARE, 30, nullptr);
                    }
                }
            }break;

            case 'i': // 異常なラベルのある画像までジャンプ 
            case 'I':
            {
                if (ctrl) 
                {
                    SaveLabelsToFileSingle(hWnd, GP.imgIdx, 0.0f);
                    PostMessage(hWnd, WM_COMMAND, ID_ANNOT_JUMP_IGNOREBOX, 0); // ジャンプメニューを呼び出す
                }
            }break;

			case 'z': // Undo
			case 'Z':
            {
                if(ctrl)
                {
                    if (GP.imgIdx < GP.imgObjs.size())
                    {
                        // 画像が存在する場合
                        //UndoClear();
                        //InvalidateRect(hWnd, NULL, TRUE); // 再描画
						PostMessageW(hWnd, WM_COMMAND, IDM_UNDO_CLEAR_LABELS, 0); // Undoメニューを呼び出す
                    }
                }
            }break;

            case '0': // 自動アノテーション
            {
                //if (ctrl && shift) { //上手くいかない
                if (ctrl) {
                    PostMessageW(hWnd, WM_COMMAND, IDM_EDIT_CLEAR_LABELS, 0); // 自動アノテーションメニューを呼び出す
                }
            }break;

            case '1': // 自動アノテーション
            {
                //GDNNP.yolo.confThreshold = 0.1f;
                if(ctrl){
                    int idx = (int)wParam - '1';
                    AutoConfirmByKey(idx);
                }else{
                    GDNNP.yolo.confThreshold = GP.autoConfirmThresh[0];
                    PostMessageW(hWnd, WM_COMMAND, IDM_YOLO_PRESETBOX, 0); // 自動アノテーションメニューを呼び出す
                }
            }break;

            case '2': // 自動アノテーション 高感度
            {
                //GDNNP.yolo.confThreshold = 0.01f;
                if (ctrl) {
                    int idx = (int)wParam - '1';
                    AutoConfirmByKey(idx);
                }else {
                    GDNNP.yolo.confThreshold = GP.autoConfirmThresh[1];
                    PostMessageW(hWnd, WM_COMMAND, IDM_YOLO_PRESETBOX, 0); // 自動アノテーションメニューを呼び出す
                }
            }break;
            case '3': // 自動アノテーション 超高感度
            {
                if (ctrl) {
                    int idx = (int)wParam - '1';
                    AutoConfirmByKey(idx);
                }else {
                    //GDNNP.yolo.confThreshold = 0.005f;
                    GDNNP.yolo.confThreshold = GP.autoConfirmThresh[2];
                    PostMessageW(hWnd, WM_COMMAND, IDM_YOLO_PRESETBOX, 0); // 自動アノテーションメニューを呼び出す
                }
            }break;
            case '4': // 自動アノテーション 超々高感度
            {
                if (ctrl) {
                    int idx = (int)wParam - '1';
                    AutoConfirmByKey(idx);
                }else {
                    //GDNNP.yolo.confThreshold = 0.001f;
                    GDNNP.yolo.confThreshold = GP.autoConfirmThresh[3];
                    PostMessageW(hWnd, WM_COMMAND, IDM_YOLO_PRESETBOX, 0); // 自動アノテーションメニューを呼び出す
                }
            }break;
            case '5': // 自動アノテーション 超々々高感度
            {
                if (ctrl) {
                    int idx = (int)wParam - '1';
                    AutoConfirmByKey(idx);
                } else {
                    //GDNNP.yolo.confThreshold = 0.0001f;
                    GDNNP.yolo.confThreshold = GP.autoConfirmThresh[4];
                    PostMessageW(hWnd, WM_COMMAND, IDM_YOLO_PRESETBOX, 0); // 自動アノテーションメニューを呼び出す
                }
            }break;
            case '6': // 自動アノテーション 超々々高感度
            {
                if (ctrl) {
                    int idx = (int)wParam - '1';
                    AutoConfirmByKey(idx);
                } else {
                    //GDNNP.yolo.confThreshold = 0.000005f;
                    GDNNP.yolo.confThreshold = GP.autoConfirmThresh[5];
                    PostMessageW(hWnd, WM_COMMAND, IDM_YOLO_PRESETBOX, 0); // 自動アノテーションメニューを呼び出す
                }
            }break;
            case '7': // 自動アノテーション 超々々高感度
            {
                if (ctrl) {
                    int idx = (int)wParam - '1';
                    AutoConfirmByKey(idx);
                } else {
                    //GDNNP.yolo.confThreshold = 0.0000005f;
                    GDNNP.yolo.confThreshold = GP.autoConfirmThresh[6];
                    PostMessageW(hWnd, WM_COMMAND, IDM_YOLO_PRESETBOX, 0); // 自動アノテーションメニューを呼び出す
                }
            }break;

            case 'Q': // バウンディングボックスの表示を切り替え
            case 'q':
            {
                if(GP.isShowBbox)
					GP.isShowBbox = false;
                else
					GP.isShowBbox = true;
            }break;
        }
        InvalidateRect(hWnd, NULL, TRUE); // 再描画
    }
    break;


    case WM_TIMER:
	{
        //画像比較用のタイマーの処理
        if (wParam == IDT_COMPARE)
        {   
			if (GP.imgIdxCompare != GP.imgIdx) // 比較用の画像インデックスが現在の画像と異なる場合
				GP.imgIdxCompare = GP.imgIdx;
            else
                GP.imgIdxCompare = GP.imgIdx+1;

            InvalidateRect(hWnd, nullptr, FALSE);
        }
        return 0;
	}
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }


    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

