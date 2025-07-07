    // CPP_Anotation.cpp : アプリケーションのエントリ ポイントを定義します。

#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFunctions.h"
#include "CPP_Anotation6.h"

// 必要なライブラリ
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
    
    //コマンドライン引数
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

    wcex.style = CS_HREDRAW | CS_VREDRAW;
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
void ShowClassPopupMenu(HWND hWnd);
///////////////////////////////////////////////////////////////////////
//ラベルのクラス名をポップアップメニューで表示する関数 編集用
void ShowClassPopupMenu_for_Edit(HWND hWnd, int activeObjectIDX);

///////////////////////////////////////////////////////////////////////
int CreatePopupMenuFor_Labels_in_CurrentImage(HWND hWnd);

/////////////////////////////////////////////////////////////////////////
void CheckMenu(HWND hWnd, int _IDM, bool _sw);

/////////////////////////////////////////////////////////////////////////
//未ラベルの画像までジャンプする関数
void JumpToUnlabeledImage(HWND hWnd);

/////////////////////////////////////////////////////////////////////////
// アノテーションデータを保存する関数 WM_Procの補助関数
// _scは0,25,50,75,100のいずれかで、0だとスケールしない
//int  SaveAnnotations(HWND hWnd, int _sc);

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
        if (GP.imgFolderPath.empty()) {
            GP.imgObjs.clear(); // 画像ファイルのパスと矩形の配列
            GP.imgFolderPath = INIT_IMGFOLDER;
            LoadImageFiles(GP.imgFolderPath, GP.imgObjs);
        }
        //GP.imgObjs.clear(); // 画像ファイルのパスと矩形の配列
        //GP.imgFolderPath = INIT_IMGFOLDER; // フォルダパスを指定
        //LoadImageFiles(GP.imgFolderPath, GP.imgObjs); // フォルダ内の画像ファイルを取得
        SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, 0, (int)GP.imgObjs.size()); // タイトルバーに画像のパスを表示

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

                // フォルダ選択ダイアログを表示
                if (!_folderpath.empty()) 
                {
                    GP.labelFolderPath = _folderpath; // フォルダパスを指定

                    // フォルダが選択された場合、アノテーションデータを読み込み
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
			    _filepath = GetFileName(hWnd, L"クラス分類ファイルを読込", 0);
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
					float(GP.MINSIZEW) / float(GP.IMGSIZEW),
                    float(GP.MINSIZEH) / float(GP.IMGSIZEH));

                if (result.has_value())
                {
                    // 前の画像に移動する前に、現在のラベルを保存
                    // 自動保存の場合は全て保存
                    SaveLabelsToFileSingle(hWnd, GP.imgIdx, 0.0f);
                    // 
                    GP.imgIdx = result.value() % GP.imgObjs.size();
                    SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.imgIdx, (int)GP.imgObjs.size()); // タイトルバーに画像のパスを表示
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

            // 現在のイメージのラベリングされたオブジェクトの一覧のポップアップメニューを作成する
            case IDM_PMENU_LABEL_BASE:
			    CreatePopupMenuFor_Labels_in_CurrentImage(hWnd);
                break;

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
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

#ifdef ORG_WMPAINT
        // マウス移動中でない場合は、描画処理を行う
        if (!GP.isMouseMoving)
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // メモリDC作成
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, GP.width, GP.height);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            // GDI+ の Graphics を memDC に結びつける
            Graphics graphics(memDC);
            graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
            graphics.SetSmoothingMode(SmoothingModeAntiAlias); // 任意

            // 背景塗りつぶし（必要に応じて）
            graphics.Clear(Color(0, 0, 0)); // 黒背景

            // 補間品質
            graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
            if (GP.imgObjs.size() > 0)
            {
                //画像の描画
                if (GP.imgObjs[GP.imgIdx].image) //配列が空でなければ
                {
                    //スケーリングしながら描画
                    graphics.DrawImage(GP.imgObjs[GP.imgIdx].image.get(), 0, 0, GP.width, GP.height);
                    graphics.Flush();
                }
                // 矩形を描画
                WM_PAINT_DrawLabels(graphics, GP.imgObjs[GP.imgIdx].objs, GP.width, GP.height, GP.font);
            }
            // ドラッグ中の矩形を描画
            if(GP.dgMode==DragMode::MakeBox)
                WM_PAINT_DrawTmpBox(graphics, GP.tmpLabel.Rct, GP.width, GP.height);
           
            // 最後に画面に転送
            BitBlt(hdc, 0, 0, GP.width, GP.height, memDC, 0, 0, SRCCOPY);

            // クリーンアップ
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            EndPaint(hWnd, &ps);
        }
		//すべての操作 
        GP.isMouseMoving = false; // フラグをリセット
        // 十字目盛りを描画
        DrawCrosshairLines(hWnd);
#endif
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
	// マウスの左ボタンが押されたときの処理
	case WM_LBUTTONDOWN:
	{
        // マウスの位置を取得
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hWnd, &pt);
        //矩形にマウスオーバーしていない時は矩形を作成する。
        if( GP.imgObjs[GP.imgIdx].mOverIdx == -1 )
        {
            // 矩形の開始位置を設定
            GP.tmpLabel.Rct.X = float(pt.x) / float(GP.width);
            GP.tmpLabel.Rct.Y = float(pt.y) / float(GP.height);
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
	break;
	// マウスの左ボタンが離されたときの処理
	case WM_LBUTTONUP:
	{
		//if (GP.makeBox)
        if (GP.dgMode == DragMode::MakeBox)
        {
            // マウスの位置を取得
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);
            ReleaseCapture(); // マウスキャプチャを解放

            // 矩形の幅と高さを計算
            GP.tmpLabel.Rct.Width = float(pt.x) / float(GP.width) - GP.tmpLabel.Rct.X;
            GP.tmpLabel.Rct.Height = float(pt.y) / float(GP.height) - GP.tmpLabel.Rct.Y;
            NormalizeRect(GP.tmpLabel.Rct); // 矩形の座標を正規化

			//クラス名をポップアップメニューで表示
            ShowClassPopupMenu(hWnd);

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
	case WM_RBUTTONDOWN:
	{
		// マウスの位置を取得
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hWnd, &pt);
		// ヒット中の矩形のインデックスを保存
		GP.activeIdx = GP.imgObjs[GP.imgIdx].mOverIdx;
		if (GP.activeIdx != -1)
		{
			// クラス名をポップアップメニューで表示
			ShowClassPopupMenu_for_Edit(hWnd, GP.activeIdx);
		}
	}
    // マウスの移動中の処理
    case WM_MOUSEMOVE:
    {
        // マウスの位置を取得
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hWnd, &pt);


        //if (GP.makeBox)
        if (GP.dgMode == DragMode::MakeBox)
        {
            GP.tmpLabel.Rct.Width = float(pt.x) / float(GP.width) - GP.tmpLabel.Rct.X;
            GP.tmpLabel.Rct.Height = float(pt.y) / float(GP.height) - GP.tmpLabel.Rct.Y;

            // 再描画
            InvalidateRect(hWnd, NULL, TRUE);
        }
        else if (GP.dgMode == DragMode::ReBox) // 矩形の編集モード
        {
            // マウス移動分を計算
            float dx = float(pt.x) - GP.prevMouse.X;
            float dy = float(pt.y) - GP.prevMouse.Y;
			if (!GP.imgObjs[GP.imgIdx].objs.empty())//objsが空でない場合 
            {
                auto& r = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct;
                GP.imgObjs[GP.imgIdx].isEdited = true; // 編集されたことにする
                switch (GP.edMode)
                {
                case EditMode::Top:
                    // 上辺を dy だけ動かす → 矩形の高さは −dy だけ変わる
                    r.Y += dy / GP.height;
                    r.Height -= dy / GP.height;
                    break;
                case EditMode::Bottom:
                    // 下辺を dy だけ動かす → 矩形の高さは +dy だけ変わる
                    r.Height += dy / GP.height;
                    break;
                case EditMode::Left:
                    // 左辺を dx だけ動かす → 矩形の幅は −dx だけ変わる
                    r.X += dx / GP.width;
                    r.Width -= dx / GP.width;
                    break;
                case EditMode::Right:
                    // 右辺を dx だけ動かす → 矩形の幅は +dx だけ変わる
                    r.Width += dx / GP.width;
                    break;
                case EditMode::LeftTop:
                    // 左上を dx,dy だけ動かす → 矩形の幅は −dx だけ変わる
                    // 矩形の高さは −dy だけ変わる
                    r.X += dx / GP.width;
                    r.Y += dy / GP.height;
                    r.Width -= dx / GP.width;
                    r.Height -= dy / GP.height;
                    break;
                case EditMode::RightTop:
                    // 右上を dx,dy だけ動かす → 矩形の幅は +dx だけ変わる
                    // 矩形の高さは −dy だけ変わる
                    r.Y += dy / GP.height;
                    r.Width += dx / GP.width;
                    r.Height -= dy / GP.height;
                    break;
                case EditMode::LeftBottom:
                    // 左下を dx,dy だけ動かす → 矩形の幅は −dx だけ変わる
                    // 矩形の高さは +dy だけ変わる
                    r.X += dx / GP.width;
                    r.Height += dy / GP.height;
                    r.Width -= dx / GP.width;
                    break;
                case EditMode::RightBottom:
                    // 右下を dx,dy だけ動かす → 矩形の幅は +dx だけ変わる
                    // 矩形の高さは +dy だけ変わる
                    r.Width += dx / GP.width;
                    r.Height += dy / GP.height;
                    break;
                default:
                    break;
                }
                // マウス位置を保存
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
                        GetIdxMouseOnRectEdge(pt, curImg.objs,
                            GP.edMode, GP.Overlap);
                }
            }

            switch (GP.edMode)
            {
            case EditMode::Top:
                // 上辺をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y = float(pt.y) / float(GP.height);
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Height = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y - GP.prevMouse.Y;
                break;
            case EditMode::Bottom:
                // 下辺をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Height = float(pt.y) / float(GP.height) - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y;
                break;
            case EditMode::Left:
                // 左辺をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X = float(pt.x) / float(GP.width);
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Width = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X - GP.prevMouse.X;
                break;
            case EditMode::Right:
                // 右辺をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Width = float(pt.x) / float(GP.width) - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X;
                break;
            case EditMode::LeftTop:
                // 左上をドラッグ。 直接、マウスの値を辺の位置に設定   
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X = float(pt.x) / float(GP.width);
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y = float(pt.y) / float(GP.height);
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Width = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X - GP.prevMouse.X;
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Height = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y - GP.prevMouse.Y;
                break;
            case EditMode::RightTop:
                // 右上をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y = float(pt.y) / float(GP.height);
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Width = float(pt.x) / float(GP.width) - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X;
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Height = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y - GP.prevMouse.Y;
                break;
            case EditMode::LeftBottom:
                // 左下をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X = float(pt.x) / float(GP.width);
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Height = float(pt.y) / float(GP.height) - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y;
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Width = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X - GP.prevMouse.X;
                break;
            case EditMode::RightBottom:
                // 右下をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Width = float(pt.x) / float(GP.width) - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.X;
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Height = float(pt.y) / float(GP.height) - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].Rct.Y;
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
        else
        {
			// マウスカーソルと矩形が重なっていないかどうかを調べて、
            // 重なっていたらそのインデックスを取得し、現在アクティブな画像のmOverIdxに格納
            if (GP.imgIdx < GP.imgObjs.size()) {              // ← これが先
                auto& curImg = GP.imgObjs[GP.imgIdx];         // 安全に参照取得
                if (!curImg.objs.empty()) {
                    curImg.mOverIdx =
                        GetIdxMouseOnRectEdge(pt, curImg.objs,
                            GP.edMode, GP.Overlap);
                }
            }

            InvalidateRect(hWnd, NULL, TRUE);
        }
    }
    break;

    //キー入力を処理する
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
            }
            break;

            case 'i': // 異常なラベルのある画像までジャンプ 
            case 'I':
            {
                if (ctrl) 
                {
					PostMessage(hWnd, WM_COMMAND, ID_ANNOT_JUMP_IGNOREBOX, 0); // ジャンプメニューを呼び出す
                }
            }
            break;
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

///////////////////////////////////////////////////////////////////////
// LabelObjを描画する関数
void WM_PAINT_DrawLabels(
    Gdiplus::Graphics& graphics,
    const std::vector<LabelObj>& objs,
    int clientWidth,
    int clientHeight,
    Gdiplus::Font* font
)
{
    int _idx = 0;
    for (const auto& obj : objs)
    {
        // ペン幅はマウスオーバーで太く
        int penWidth = obj.penWidth + (obj.mOver ? 2 : 0);
        Gdiplus::Pen pen(obj.color, static_cast<REAL>(penWidth));
        pen.SetDashStyle(obj.dashStyle);

        if (isIgnoreBox(obj, GP.MINSIZEW / GP.IMGSIZEW, GP.MINSIZEH / GP.IMGSIZEH))
        {
            //太く、さらにマウスオーバーで太く
            int penWidth = obj.penWidth + 2 + (obj.mOver ? 2 : 0);
            pen.SetWidth(static_cast<REAL>(penWidth));
            //色は赤にする
			pen.SetColor(Gdiplus::Color(255, 0, 0)); // 赤色
			//ソリッド
			pen.SetDashStyle(Gdiplus::DashStyleSolid); // ソリッド
        }

        // 矩形の座標をピクセル変換
		float x0 = obj.Rct.X * clientWidth;
        float y0 = obj.Rct.Y * clientHeight;
        float w = obj.Rct.Width * clientWidth;
        float h = obj.Rct.Height * clientHeight;

        // 矩形描画
        graphics.DrawRectangle(&pen, x0, y0, w, h);

        // ラベル文字描画
        Gdiplus::SolidBrush textBrush(obj.color);
        const std::wstring& text = L"(" + std::to_wstring(_idx) + L")" + obj.ClassName;

        // 文字高さを測定
        Gdiplus::RectF textBounds;
        graphics.MeasureString(
            text.c_str(), -1,
            font,
            Gdiplus::PointF(0, 0),
            &textBounds
        );
        float textHeight = textBounds.Height;

        // 矩形の上部外側にオフセット
        Gdiplus::PointF textPos(x0, y0 - textHeight - 2.0f);
        graphics.DrawString(
            text.c_str(), -1,
            font,
            textPos,
            &textBrush
        );
        _idx++;
    }
}

///////////////////////////////////////////////////////////////////////
// 一時的な矩形を描画する関数
void WM_PAINT_DrawTmpBox(
    Gdiplus::Graphics& graphics,
    const Gdiplus::RectF& normRect,
    int clientWidth,
    int clientHeight
)
{
    // ペンは赤・太さ2px
    Gdiplus::Pen pen(Gdiplus::Color(255, 0, 0), 2.0f);

    // 正規化矩形をローカル変数にコピー
    float x = normRect.X;
    float y = normRect.Y;
    float w = normRect.Width;
    float h = normRect.Height;

    // 幅・高さが負の場合は方向を反転
    if (w < 0) { x += w; w = -w; }
    if (h < 0) { y += h; h = -h; }

    // ピクセル座標に変換して描画
    graphics.DrawRectangle(
        &pen,
        x * clientWidth,
        y * clientHeight,
        w * clientWidth,
        h * clientHeight
    );
}
///////////////////////////////////////////////////////////////////////
// マウスカーソルの位置に十字線を描画する関数
void DrawCrosshairLines(HWND hWnd)
{
    // マウス位置をクライアント座標で取得
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(hWnd, &pt);

    // クライアント領域サイズを取得
    RECT rect;
    GetClientRect(hWnd, &rect);

    // GDI で十字線を XOR 描画
    HDC hdc = GetDC(hWnd);
    SetROP2(hdc, R2_XORPEN);
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HGDIOBJ oldPen = SelectObject(hdc, hPen);

    // 水平線
    MoveToEx(hdc, 0, pt.y, NULL);
    LineTo(hdc, rect.right, pt.y);
    // 垂直線
    MoveToEx(hdc, pt.x, 0, NULL);
    LineTo(hdc, pt.x, rect.bottom);

    // 後始末
    SelectObject(hdc, oldPen);
    DeleteObject(hPen);
    ReleaseDC(hWnd, hdc);
}


///////////////////////////////////////////////////////////////////////
// ラベルのクラス名をポップアップメニューで表示する関数の共通部分
int ShowClassPopupMenu_Core(HWND hWnd, UINT& cmd)
{
    const int _perColumn = 20;

    // ポップアップメニューの作成
    HMENU hPopup = CreatePopupMenu();
    if (!hPopup) return 0;

    // カーソル位置を取得（画面→クライアント座標は不要）
    POINT pt;
    GetCursorPos(&pt);

    // 先頭で未使用アクセラレータを管理するセットを用意
    std::set<wchar_t> usedAccels;

    // メニュー項目を追加 
    for (size_t i = 0; i < GP.ClsNames.size(); ++i)
    {
        const std::wstring& name = GP.ClsNames[i];
        wchar_t base = name[0];          // 元の頭文字
#if defined(USE_ACCEL_NUMBER) //9まで使い切ってしまうとnullになり、メニュー表示が異常になる
        wchar_t accel = 0;               // 最終的に使うアクセラレータ
        // まだ使われていなければその文字を使う
        if (usedAccels.insert(base).second) {
            accel = base;
        }
        else {
            // 重複していたら '1'～'9' から未使用のものを探す
            for (wchar_t d = L'1'; d <= L'9'; ++d) {
                if (usedAccels.insert(d).second) {
                    accel = d;
                    break;
                }
            }
            // ※9個以上の重複があり得るなら、ここを拡張してください
        }
#else
        wchar_t accel = base; // ショートカットキーは元の頭文字
#endif
        // ショートカットキー付き文字列を作成
        std::wostringstream clsName;
        clsName << L"(&" << accel << L") " << name;

        // 分類がたくさんあるときの処理
        UINT flags = MF_STRING;
        if (i > 0 && (i % _perColumn) == 0) {
            flags |= MF_MENUBREAK;
        }

        AppendMenuW(
            hPopup,
            flags,
            IDM_PMENU_CLSNAME00 + static_cast<UINT>(i),
            clsName.str().c_str()
        );
    }

    AppendMenuW(hPopup, MF_STRING,
        IDM_PMENU_CLSNAME00 + static_cast<UINT>(GP.ClsNames.size()),
        L"DELETE(&1)");

    AppendMenuW(hPopup, MF_STRING,
        IDM_PMENU_CLSNAME00 + static_cast<UINT>(GP.ClsNames.size()) + 1,
        L"CANCEL");

    // ウィンドウを前面にしてから表示
    SetForegroundWindow(hWnd);
    cmd = TrackPopupMenuEx(
        hPopup,
        TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
        pt.x, pt.y,
        hWnd,
        NULL
    );
    DestroyMenu(hPopup);

    return 1; // 選択されたコマンドを返す
}


///////////////////////////////////////////////////////////////////////
// ラベルのクラス名をポップアップメニューで表示する関数
// 関数定義（例えば DrawingHelpers.cpp などにまとめてもOK）
void ShowClassPopupMenu(HWND hWnd)
{
    //UINT cmd = 0;
    //int _cmd = ShowClassPopupMenu_Core(hWnd);
    //if (_cmd != -1) // コマンドが正常に取得できた場合
    //    cmd = static_cast<UINT>(_cmd); // コマンドをUINTに変換
    //else
    //    return; // メニューの表示に失敗した場合は何もしない
    UINT cmd = 0;
    if(ShowClassPopupMenu_Core(hWnd,cmd))
    {
        // 選択結果を反映
        if (cmd >= IDM_PMENU_CLSNAME00 &&
            cmd < IDM_PMENU_CLSNAME00 + GP.ClsNames.size())
        {
            GP.selectedClsIdx = cmd - IDM_PMENU_CLSNAME00;

            // tmpLabel に選択内容を設定
            GP.tmpLabel.ClassName = GP.ClsNames[GP.selectedClsIdx];
            GP.tmpLabel.ClassNum = GP.selectedClsIdx;
            GP.tmpLabel.color = GP.ClsColors[GP.selectedClsIdx];
            GP.tmpLabel.dashStyle = GP.ClsDashStyles[GP.selectedClsIdx];
            GP.tmpLabel.penWidth = GP.ClsPenWidths[GP.selectedClsIdx];

            // オブジェクトを登録
            if (!GP.imgObjs.empty())
            {
                GP.imgObjs[GP.imgIdx].objs.push_back(GP.tmpLabel);
                GP.imgObjs[GP.imgIdx].isEdited = true; // 編集フラグを立てる
            }
        }
        else if (cmd == 0 || cmd == IDM_PMENU_CLSNAME00 + static_cast<UINT>(GP.ClsNames.size())) // CANCEL
        {
            return;
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////
// ラベルのクラス名をポップアップメニューで表示する関数
// 関数定義（例えば DrawingHelpers.cpp などにまとめてもOK）
void ShowClassPopupMenu_for_Edit(HWND hWnd, int activeObjectIDX )
{
    UINT cmd = 0;
    if (ShowClassPopupMenu_Core(hWnd, cmd))
    {
        // 選択結果を反映
        if (cmd >= IDM_PMENU_CLSNAME00 &&
            cmd < IDM_PMENU_CLSNAME00 + GP.ClsNames.size())
        {
            GP.selectedClsIdx = cmd - IDM_PMENU_CLSNAME00;

            // activeObjectIDXで示すオブジェクトに選択内容を上書き
            GP.imgObjs[GP.imgIdx].objs[activeObjectIDX].ClassName = GP.ClsNames[GP.selectedClsIdx];
            GP.imgObjs[GP.imgIdx].objs[activeObjectIDX].ClassNum = GP.selectedClsIdx;
            GP.imgObjs[GP.imgIdx].objs[activeObjectIDX].color = GP.ClsColors[GP.selectedClsIdx];
            GP.imgObjs[GP.imgIdx].objs[activeObjectIDX].dashStyle = GP.ClsDashStyles[GP.selectedClsIdx];
            GP.imgObjs[GP.imgIdx].objs[activeObjectIDX].penWidth = GP.ClsPenWidths[GP.selectedClsIdx];

            GP.imgObjs[GP.imgIdx].isEdited = true; // 編集フラグを立てる
        }
        else if (cmd == IDM_PMENU_CLSNAME00 + static_cast<UINT>(GP.ClsNames.size())) // DELETE
        {
            // オブジェクトを削除
            GP.imgObjs[GP.imgIdx].objs.erase(GP.imgObjs[GP.imgIdx].objs.begin() + activeObjectIDX);
            GP.imgObjs[GP.imgIdx].isEdited = true; // 編集フラグを立てる
        }
        else if (cmd == IDM_PMENU_CLSNAME00 + static_cast<UINT>(GP.ClsNames.size()) + 1) // CANCEL
            return;
    }

    return;
}


//////////////////////////////////////////////////////////////////////////
// 現在のイメージのオブジェクトの一覧のポップアップメニューを作成する
int CreatePopupMenuFor_Labels_in_CurrentImage(HWND hWnd)
{
    HMENU hPopup = CreatePopupMenu();
    if (!hPopup) return -1;

    POINT pt;
    GetCursorPos(&pt);

    for (size_t i = 0; i < GP.imgObjs[GP.imgIdx].objs.size(); ++i)
    {
        const auto& obj = GP.imgObjs[GP.imgIdx].objs[i];
        std::wstring menuText = L"(&" + std::to_wstring(i) + L") " + obj.ClassName;
        AppendMenuW(hPopup, MF_STRING, IDM_PMENU_LABEL00 + static_cast<UINT>(i), menuText.c_str());
    }
    AppendMenuW(hPopup, MF_STRING,
        IDM_PMENU_LABEL00 + static_cast<UINT>(GP.imgObjs[GP.imgIdx].objs.size()),
        L"(ESC)CANCEL");

    SetForegroundWindow(hWnd);
    UINT cmd = TrackPopupMenuEx(
        hPopup,
        TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
        pt.x, pt.y,
        hWnd,
        NULL
    );
    DestroyMenu(hPopup);

    //////////////////////////////////////////////////////////////////////////
    // ★ 追記部分：ラベルが選択されたら編集メニューを表示する
    if (cmd >= IDM_PMENU_LABEL00 &&
        cmd < IDM_PMENU_LABEL00 + GP.imgObjs[GP.imgIdx].objs.size())
    {
        int objIdx = static_cast<int>(cmd - IDM_PMENU_LABEL00);
        ShowClassPopupMenu_for_Edit(hWnd, objIdx);
    }
    //////////////////////////////////////////////////////////////////////////

    return static_cast<int>(cmd);
}



///////////////////////////////////////////////////////////////////////
// 描画処理を行う関数
///////////////////////////////////////////////////////////////////////
void DoPaint(HWND hWnd, WPARAM wParam, LPARAM lParam, size_t _idx)
{ 
    // マウス移動中でない場合は、描画処理を行う
    if (!GP.isMouseMoving)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // メモリDC作成
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, GP.width, GP.height);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        // GDI+ の Graphics を memDC に結びつける
        Graphics graphics(memDC);
        graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias); // 任意

        // 背景塗りつぶし（必要に応じて）
        graphics.Clear(Color(0, 0, 0)); // 黒背景

        // 補間品質
        graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        if (GP.imgObjs.size() > 0)
        {
            //画像の描画
            if (GP.imgObjs[_idx].image) //配列が空でなければ
            {
                //スケーリングしながら描画
                graphics.DrawImage(GP.imgObjs[_idx].image.get(), 0, 0, GP.width, GP.height);
                graphics.Flush();
            }
            // 矩形を描画
            WM_PAINT_DrawLabels(graphics, GP.imgObjs[_idx].objs, GP.width, GP.height, GP.font);
        }
        // ドラッグ中の矩形を描画
        if (GP.dgMode == DragMode::MakeBox)
            WM_PAINT_DrawTmpBox(graphics, GP.tmpLabel.Rct, GP.width, GP.height);

        // 最後に画面に転送
        BitBlt(hdc, 0, 0, GP.width, GP.height, memDC, 0, 0, SRCCOPY);

        // クリーンアップ
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
        EndPaint(hWnd, &ps);
    }
    //すべての操作 
    GP.isMouseMoving = false; // フラグをリセット
    // 十字目盛りを描画
    DrawCrosshairLines(hWnd);
}

///////////////////////////////////////////////////////////////////////
// ファイル保存ダイアログを表示
// この関数はWinProcの一部という位置づけ
// GP.minimumLabelSize に注意
int  SaveAnnotations(HWND hWnd, std::wstring _title, float _sc) // 最小サイズ制限（デフォルトはなし）
{
    std::wstring _folderpath;

    //すべてのラベルオブジェクトをスケーリングする
    if (_sc != 0)
    {
        for (auto& _imgObj : GP.imgObjs)
        {
            for (auto& _labelObj : _imgObj.objs)
            {
                SscalingRect(_labelObj.Rct, _labelObj.Rct_Scale, _sc, _sc);

                //if (_sc == 13) SscalingRect(_labelObj.Rct, _labelObj.Rct_Scale, 1.13, 1.13);
                //else if (_sc == 25) SscalingRect(_labelObj.Rct, _labelObj.Rct_Scale, 1.25, 1.25);
                //else if (_sc == 50) SscalingRect(_labelObj.Rct, _labelObj.Rct_Scale, 1.5, 1.5);
                //else if (_sc == 75) SscalingRect(_labelObj.Rct, _labelObj.Rct_Scale, 1.75, 1.75);
                //else if (_sc == 100) SscalingRect(_labelObj.Rct, _labelObj.Rct_Scale, 2.0, 2.0);
            }
        }
    }

    //この関数でレジストリにフォルダパスを保存をしている。
    //_folderpath = GetFolderPathIF(hWnd, GP.labelFolderPath, L"書込ラベルフォルダを選択してください"); // フォルダ選択ダイアログを表示
    _folderpath = GetFolderPathIFR(hWnd, GP.labelFolderPath, _title); // フォルダ選択ダイアログを表示

    // フォルダ選択ダイアログを表示
    if (!_folderpath.empty()) {
        int _saveok = MessageBoxW(hWnd, L"保存しますか？", L"確認", MB_OKCANCEL);
        if (_saveok == IDOK)
        {
            GP.labelFolderPath = _folderpath; // フォルダパスを指定
            //タイトルバーに編集中の画像とラベルのパスを表示
            SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.activeIdx, (int)GP.imgObjs.size()); // タイトルバーに画像のパスを表示

            // フォルダが選択された場合、アノテーションデータを保存
            for (size_t i = 0; i < GP.imgObjs.size(); i++)
            {
                // ファイル名
                std::wstring _fileName1;
                std::wstring _fileName2;

                _fileName1 = GetOnlyFileNameFormPath(GP.imgObjs[i].path);
                _fileName2 = _folderpath + L"\\" + _fileName1 + L".txt";

                bool _ret = SaveLabelsToFile(_fileName2, GP.imgObjs[i].objs, _sc, GP.minimumLabelSize, 1);
                if (!_ret)
                {
                    // 保存失敗
                    MessageBox(hWnd, L"保存失敗", L"失敗", MB_OK);
                }
            }
			if (GP.minimumLabelSize != 0.0f)
			{
				// 最小サイズ制限を設定
				GP.minimumLabelSize = 0.0f; // 保存後は最小サイズ制限をリセット
                MessageBox(hWnd, L"最小サイズ制限をリセットしました", L"お知らせ", MB_OK);
			}
        }
    }
    return 0; // 成功
}

void CheckMenu(HWND hWnd, int _IDM, bool _sw)
{
    HMENU hMenu = GetMenu(hWnd);
    UINT uState = _sw ? MF_CHECKED : MF_UNCHECKED;
    // チェック状態をセット
    CheckMenuItem(
        hMenu,
        _IDM,
        MF_BYCOMMAND |
        uState
    );
}

//未ラベルの画像までジャンプする関数
void JumpToUnlabeledImage(HWND hWnd)
{
    // ラベルのない画像までジャンプ
    for (size_t i = GP.imgIdx + 1; i < GP.imgObjs.size(); ++i)
    {
        if (GP.imgObjs[i].objs.empty())
        {
            SaveLabelsToFileSingle(hWnd, GP.imgIdx, 0.0f);

            GP.imgIdx = i; // ジャンプ


            SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.imgIdx, (int)GP.imgObjs.size()); // タイトルバーに画像のパスを表示
            InvalidateRect(hWnd, NULL, TRUE); // 再描画
            return;
        }
    }
    MessageBoxW(hWnd, L"ラベルのない画像はありません", L"情報", MB_OK);
}