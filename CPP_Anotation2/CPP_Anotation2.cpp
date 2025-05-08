// CPP_Anotation.cpp : アプリケーションのエントリ ポイントを定義します。

#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFunctions.h"
#include "CPP_Anotation2.h"

// 必要なライブラリ
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")


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

/////////////////////////////////////////////////////////////////////////
// 課題：このプログラムはメモリリークしています。
// 現象を確認してください。
// 原因を突き止めて、メモリリークを防ぐために必要な処理を追加してください。

////////////////////////////////////////////////////////////////////////　　　　
// ★★物体の種類のことを「クラシフィケーション」と呼びます。★★
////////////////////////////////////////////////////////////////////////　　　　


/////////////////////////////////////////////////////////////////////////
// 課題1: アノテーションクラスを作ってください
// ヒント 矩形データを格納するクラスを作成します。
// 
// 課題2: アノテーションクラスに、クラシフィケーションナンバー、
// 　　　 クラス名、線の色、線の幅などの要素を追加してください。
// 
// 課題3: 矩形を作成する時に、アノテーション名を付ける機能を追加してください
// ヒント: WM_LBUTTONUPイベントにポップアップメニューを表示する処理を追加します
//         アノテーションの名前リストは固定で構いません。グローバル変数に定義しておきます。
//
// 課題4: 矩形にアノテーション名を表示してください。

// [上級]
// 課題: 矩形を選択する機能を追加してください。
// 矩形を選択した時に、ポップアップメニューを表示
// 選択中の矩形の色を変更
//
// 課題: 矩形を移動する機能を追加
// 
// 課題: 矩形を削除する機能を追加


/////////////////////////////////////////////////////////////////////////
// 画像フォルダのパス
// フィルパスのフォルダは'/'で区切る必要があります。
// '\'で記述する場合は'\\'に置き換える必要があります。
const wchar_t* INIT_IMGFOLDER = L"../images/";  // JPEGまたはPNG

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
		GP.imgFolderPath = INIT_IMGFOLDER; // フォルダパスを指定
		GetImgsPaths(GP.imgFolderPath, &GP.imgPaths); // フォルダ内の画像ファイルを取得

        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 選択されたメニューの解析:
        switch (wmId)
        {
        case IDM_LOAD_IMAGES:
        {
            GP.imgFolderPath = GetFolderPath(hWnd); // フォルダ選択ダイアログを表示
            if (!GP.imgFolderPath.empty()) {
                // フォルダが選択された場合、画像ファイルを取得
                GP.imgPaths.clear();
                GetImgsPaths(GP.imgFolderPath, &GP.imgPaths); // フォルダ内の画像ファイルを取得
                GP.imgIndex = 0; // インデックスをリセット
            }
            // 再描画
            InvalidateRect(hWnd, NULL, TRUE);
        }
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
        if (image)
        {

            //スケーリングしながら描画
            graphics.DrawImage(image, 0, 0, GP.width, GP.height);
            graphics.Flush();

        }
		
        // 矩形を描画
		for (const auto& rect : GP.rects) 
        {
			Pen pen(Color(0, 128, 255), 2);
            graphics.DrawRectangle(&pen,
                rect.X * GP.width,
                rect.Y * GP.height,
                rect.Width * GP.width,
                rect.Height * GP.height
            );
		}
		
        // ドラッグ中の矩形を描画
		if (GP.makeBox) 
        {
            Pen pen(Color(255, 0, 0), 2);
            float x = GP.rect_tmp.X;
            float y = GP.rect_tmp.Y;
            float w = GP.rect_tmp.Width;
            float h = GP.rect_tmp.Height;

            if (w < 0) { x += w; w = -w; }
            if (h < 0) { y += h; h = -h; }

            graphics.DrawRectangle(&pen,
                x * GP.width,
                y * GP.height,
                w * GP.width,
                h * GP.height);
        }

        // 最後に画面に転送
        BitBlt(hdc, 0, 0, GP.width, GP.height, memDC, 0, 0, SRCCOPY);

        // クリーンアップ
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
        EndPaint(hWnd, &ps);

        //
        
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
		// 矩形の開始位置を設定
        GP.rect_tmp.X = float(pt.x) / float(GP.width);
        GP.rect_tmp.Y = float(pt.y) / float(GP.height);
		GP.rect_tmp.Width = 0;
		GP.rect_tmp.Height = 0;
		//NormalizeRect(GP.rect_tmp); // 矩形の座標を正規化

		GP.makeBox = true; // ドラッグ中フラグを立てる
		//GP.rects.push_back(rect); // 矩形を追加
		SetCapture(hWnd); // マウスキャプチャを取得
	}
	break;
	// マウスの移動中の処理
	case WM_MOUSEMOVE:
	{
		if (GP.makeBox) {
			// マウスの位置を取得
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hWnd, &pt);
            GP.rect_tmp.Width = float(pt.x) / float(GP.width) - GP.rect_tmp.X;
            GP.rect_tmp.Height = float(pt.y) / float(GP.height) - GP.rect_tmp.Y;
            //NormalizeRect(GP.rect_tmp); // 矩形の座標を正規化

            // 再描画
			InvalidateRect(hWnd, NULL, TRUE);
		}
	}
	break;

	// マウスの左ボタンが離されたときの処理
	case WM_LBUTTONUP:
	{
		if (GP.makeBox) {
			// マウスの位置を取得
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hWnd, &pt);
			// 矩形の幅と高さを計算
            GP.rect_tmp.Width = float(pt.x) / float(GP.width) - GP.rect_tmp.X;
            GP.rect_tmp.Height = float(pt.y) / float(GP.height) - GP.rect_tmp.Y;
            NormalizeRect(GP.rect_tmp); // 矩形の座標を正規化

            // 矩形を追加
			GP.rects.push_back(GP.rect_tmp);

			// テンポラリ矩形の座標をリセット
			GP.rect_tmp.X = 0;
			GP.rect_tmp.Y = 0;
			GP.rect_tmp.Width = 0;
			GP.rect_tmp.Height = 0;
			GP.makeBox = false; // ドラッグ中フラグを下ろす
			ReleaseCapture(); // マウスキャプチャを解放
			// 再描画
			InvalidateRect(hWnd, NULL, TRUE);
		}
	}
	break;


    //キー入力を処理する
	case WM_KEYDOWN:
		// swtch文で入力されたキーを判定、処理を分ける
		// swtch文が二重になっている(Windowsプログラミングでは定石)
		switch (wParam)
		{
		case 'A': // 前の画像
        case VK_LEFT:
            if (GP.imgPaths.size() > 0) {
				GP.imgIndex = (GP.imgIndex + GP.imgPaths.size() - 1) % GP.imgPaths.size();
			}
			break;
		case 'D': // 次の画像
		case VK_RIGHT:
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

