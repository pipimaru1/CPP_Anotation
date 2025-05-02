// CPP_Anotation.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "pch.h"
#include "framework.h"
#include "CPP_Anotation2.h"

#pragma comment(lib, "gdiplus.lib")

#define MAX_LOADSTRING 100

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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_GdiToken, &gdiplusStartupInput, NULL);


    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CPPANOTATION, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
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

///////////////////////////////////////////////////////
// グローバル変数の定義
// 課題ではヘッダファイルに記載だが、教材のためにここに記載します。
struct GlobalParams
{
    //ウィンドウサイズ
    RECT rect;
    int width;
    int height;

    //イメージファイル
	std::vector<std::wstring> imgPaths; // 画像ファイルのパスを格納する配列
	size_t imgIndex; // 現在の画像インデックス

    // 対象とする画像拡張子パターン
    std::vector<std::wstring> IMAGE_EXTENSIONS;
    GlobalParams();
};

///////////////////////////////////////////////////
//コンストラクタ
GlobalParams::GlobalParams()
	:IMAGE_EXTENSIONS{	L"*.jpg", L"*.jpeg", L"*.png", L"*.bmp", L"*.gif"}
{
	rect.left = 0;
	rect.top = 0;
	rect.right = 0;
	rect.bottom = 0;
	width = 0;
	height = 0;

	imgPaths.clear();
	imgIndex = 0;
}


// グローバル変数のインスタンスを作成
GlobalParams GP;

// 画像ファイルのパスの配列を取得する関数
int GetImgsPaths(const std::wstring& folderPath, std::vector<std::wstring>* imagePaths);


// フィルパスのフォルダは'/'で区切る必要があります。
// '\'で記述する場合は'\\'に置き換える必要があります。
const wchar_t* g_imagePath = L"./Image01.jpg";  // JPEGまたはPNG
const wchar_t* g_imageFolder = L"../images/";  // JPEGまたはPNG

/////////////////////////////////////////////////////////////////////////
// 課題：ウィンドウのサイズ変更時に画像をリサイズしてください
// ヒント：WM_SIZEメッセージを処理し、GetClientRect関数を使用してウィンドウのクライアント領域のサイズを取得します。

/////////////////////////////////////////////////////////////////////////
// 課題: 全てのグローバル変数は、専用のクラスで定義してください。
// マラス定義はヘッダーファイルに記述し、他のCPPからも参照できるようにします。
// グローバル変数に画像の幅と高さを格納し、ウィンドウのサイズに合わせて画像をリサイズします。

// 課題 :マウスドラッグで矩形を描画する機能を追加してください。
// 　ヒント：WM_LBUTTONDOWN、WM_MOUSEMOVE、WM_LBUTTONUPメッセージを処理して、
// 　マウスの左ボタンが押されたとき、移動中、ボタンが離されたときの処理を行います。

// 課題: 任意の画像ファイルを読み込む機能を追加してください。

// 課題 :キー入力で次の画像と前の画像に切り替える機能を追加してください。
// 　ヒント：WM_KEYDOWNメッセージを処理して、キー入力を検出します。
// 　キーAとキーDを使用して、前の画像と次の画像に切り替えます。
// 　画像ファイルのパスを配列に格納し、インデックスを管理します。


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
		GP.imgPaths.clear();
		GetImgsPaths(g_imageFolder, &GP.imgPaths); // フォルダ内の画像ファイルを取得

        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 選択されたメニューの解析:
        switch (wmId)
        {
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
    case WM_PAINT:
    {
        static Image* image = nullptr;
		if (GP.imgPaths.size() > 0)
		{
			// 画像のパスを取得
            image = new Image(GP.imgPaths[GP.imgIndex].c_str()); // 絶対パスを指定
        }
        else
		{
			// 画像が見つからない場合は、NO Image
			image = new Image(L"NO Image");
		}

        if (image)
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

            //スケーリングしながら描画
            graphics.DrawImage(image, 0, 0, GP.width, GP.height);
            graphics.Flush();

            // 最後に画面に転送
            BitBlt(hdc, 0, 0, GP.width, GP.height, memDC, 0, 0, SRCCOPY);

            // クリーンアップ
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            EndPaint(hWnd, &ps);
        }
        break;
    }
    break;

    //バッグラウンド描画を無効にする
    case WM_ERASEBKGND: 
        return 1;  // 背景の塗りつぶし処理を行った（何もしないけど）ことをOSに伝える

    case WM_SIZE:
    {
        // ウィンドウのサイズ変更時の処理
        //RECT rect;
        GetClientRect(hWnd, &GP.rect);
        GP.width = GP.rect.right - GP.rect.left;
        GP.height = GP.rect.bottom - GP.rect.top;
        // 画像をウィンドウのサイズに合わせて描画する場合は、ここで処理を追加
    }
    break;

	case WM_KEYDOWN:
		// キー入力処理
		switch (wParam)
		{
		case 'A': // 前の画像
			if (GP.imgPaths.size() > 0) {
				GP.imgIndex = (GP.imgIndex + GP.imgPaths.size() - 1) % GP.imgPaths.size();
			}
			break;
		case 'D': // 次の画像
			if (GP.imgPaths.size() > 0) {
				GP.imgIndex = (GP.imgIndex + 1) % GP.imgPaths.size();
			}
			break;
		}
		InvalidateRect(hWnd, NULL, TRUE); // 再描画
		break;

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
// 画像ファイルかどうかを判定する関数
bool IsImageFile(const std::wstring& fileName)
{
    for (const auto& pattern : GP.IMAGE_EXTENSIONS) {
        if (PathMatchSpecW(fileName.c_str(), pattern.c_str())) {
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////
// フォルダの画像ファイルを取得する関数
int GetImgsPaths(const std::wstring& folderPath, std::vector<std::wstring>* imagePaths)
{
    imagePaths->clear();

    std::wstring searchPath = folderPath;
    if (!searchPath.empty() && searchPath.back() != L'\\')
        searchPath += L'\\';
    searchPath += L"*.*";  // 全ファイル対象

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0; // フォルダが見つからない
    }

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wstring fileName = findData.cFileName;
            if (IsImageFile(fileName)) {
                std::wstring fullPath = folderPath;
                if (!fullPath.empty() && fullPath.back() != L'\\')
                    fullPath += L'\\';
                fullPath += fileName;
                imagePaths->push_back(fullPath);
            }
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    return static_cast<int>(imagePaths->size());
}