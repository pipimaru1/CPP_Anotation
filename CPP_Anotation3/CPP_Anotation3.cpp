﻿// CPP_Anotation.cpp : アプリケーションのエントリ ポイントを定義します。

#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFunctions.h"
#include "CPP_Anotation3.h"

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
// 課題5: 画像ファイルと矩形を紐付けてください。
// 　　　 画像が変わると表示する矩形を変えるようにします。
// ヒント: 画像データを含むクラスを作成します。
//         そのクラスに、画像のパスと矩形の配列を持たせます。
//         さらにそのクラスを配列にします。
//
// 課題6: 物体クラスの配列をファイル保存する機能を追加してください。
// ヒント: 物体クラスから文字列を生成する関数を作ります。
// 　　　　メンバ関数でも良いです。両方作るのはお薦めしない。
// 　　　　そのあと配列を入力値とするファイル書き込み関数を作ります。
//         出力文字列はutf-8なのでstd::stringを使います。
// 大ヒント: copilotを使う場合は先に関数の説明文を記述します。
//
// ファイル形式は下記のYOLOV3形式を参考にしてください。
// 1 0.341927 0.748148 0.111979 0.222222
// 12 0.700781 0.430556 0.091146 0.118519
// 1 0.579688 0.356019 0.057292 0.102778
// 6 0.441406 0.303704 0.144271 0.231481
// 4 0.234635 0.312037 0.111979 0.225926
// １列目はクラス番号、2列目はx座標、3列目はy座標、4列目は幅、5列目は高さです。
// 1列目は0から始まる整数、2列目から5列目は0.0から1.0の実数で、小数点以下6桁の精度で保存します。
// 区切りは半角スペースです。
// 文字コードはUTF-8で保存してください。


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
		GetImgsPaths(GP.imgFolderPath, GP.imgPaths); // フォルダ内の画像ファイルを取得

        break;

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
        case IDM_LOAD_IMAGES:
        {
            GP.imgFolderPath = GetFolderPath(hWnd); // フォルダ選択ダイアログを表示
            if (!GP.imgFolderPath.empty()) {
                // フォルダが選択された場合、画像ファイルを取得
                GP.imgPaths.clear();
                GetImgsPaths(GP.imgFolderPath, GP.imgPaths); // フォルダ内の画像ファイルを取得
                GP.imgIdx = 0; // インデックスをリセット
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
        ///////////////////
        //画像の描画
        static Image* image = nullptr;
		if (GP.imgPaths.size() > 0)
		{
			// 画像のパスを取得
            image = new Image(GP.imgPaths[GP.imgIdx].c_str()); // 絶対パスを指定
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
		
        ///////////////////
        // 矩形を描画
		for (const auto& _obj : GP.objs) 
        {
            //Pen pen(Color(0, 128, 255), 2);
			Pen pen(_obj.color, _obj.penWidth);
			pen.SetDashStyle(_obj.dashStyle);

            float x0 = _obj.rect.X * GP.width;
            float y0 = _obj.rect.Y * GP.height;
            float w = _obj.rect.Width * GP.width;
            float h = _obj.rect.Height * GP.height;

            graphics.DrawRectangle(&pen, x0, y0, w, h);

            // テキストと同色のブラシを作る
            Gdiplus::SolidBrush textBrush(_obj.color);

            const std::wstring text = _obj.ClassName;

            // 文字列の大体の高さを取得
            RectF   textBounds;
            graphics.MeasureString(text.c_str(), -1, GP.font, PointF(0, 0), &textBounds);
            float textHeight = textBounds.Height;

            // テキストを矩形の左上外側にオフセット
            PointF  textPos(x0, y0 - textHeight - 2.0f);
            // 文字列描画
            graphics.DrawString(text.c_str(), -1, GP.font, textPos, &textBrush);

		}
		
        /////////////////////////
        // ドラッグ中の矩形を描画
		if (GP.makeBox) 
        {
            Pen pen(Color(255, 0, 0), 2);
            float x = GP.obj_tmp.rect.X;
            float y = GP.obj_tmp.rect.Y;
            float w = GP.obj_tmp.rect.Width;
            float h = GP.obj_tmp.rect.Height;

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

        ///メモリの開放
		delete image; 
		image = nullptr; // 念のためnullにする

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
        GP.obj_tmp.rect.X = float(pt.x) / float(GP.width);
        GP.obj_tmp.rect.Y = float(pt.y) / float(GP.height);
		GP.obj_tmp.rect.Width = 0;
		GP.obj_tmp.rect.Height = 0;

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
            GP.obj_tmp.rect.Width = float(pt.x) / float(GP.width) - GP.obj_tmp.rect.X;
            GP.obj_tmp.rect.Height = float(pt.y) / float(GP.height) - GP.obj_tmp.rect.Y;

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
            ReleaseCapture(); // マウスキャプチャを解放

            // 矩形の幅と高さを計算
            GP.obj_tmp.rect.Width = float(pt.x) / float(GP.width) - GP.obj_tmp.rect.X;
            GP.obj_tmp.rect.Height = float(pt.y) / float(GP.height) - GP.obj_tmp.rect.Y;
            NormalizeRect(GP.obj_tmp.rect); // 矩形の座標を正規化

            HMENU hPopup = CreatePopupMenu();
            if (hPopup)
            {
                //マウスカーソルの位置を取得
                POINT pt2;
                GetCursorPos(&pt2);

                //ホップアップメニューの作成
                for (size_t i = 0; i < GP.ClsNames.size(); ++i)
                {
                    AppendMenuW(hPopup, MF_STRING, IDM_PMENU_CLSNAME00 + (UINT)i,
                        GP.ClsNames[i].c_str());
                }
                AppendMenuW(hPopup, MF_STRING,
                    IDM_PMENU_CLSNAME00 + (UINT)GP.ClsNames.size(),
                    L"CANCEL");

                //念のためウィンドウを手前に
                SetForegroundWindow(hWnd);

                // ポップアップメニューの表示
                UINT cmd = TrackPopupMenuEx(
                    hPopup,
                    TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                    pt2.x, pt2.y,
                    hWnd,
                    NULL
                );
                // ポップアップメニューの破棄
                DestroyMenu(hPopup);

                // 戻り値 cmd で即時処理
                if (cmd >= IDM_PMENU_CLSNAME00 && cmd < (IDM_PMENU_CLSNAME00 + GP.ClsNames.size()))
                {
                    GP.selectedClsIdx = cmd - IDM_PMENU_CLSNAME00;

					// 選択されたクラシフィケーションのインデックスを取得 その他属性の設定
                    GP.obj_tmp.ClassName = GP.ClsNames[GP.selectedClsIdx];
					GP.obj_tmp.CalassNum = GP.selectedClsIdx;
					GP.obj_tmp.color = GP.ClsColors[GP.selectedClsIdx];
					GP.obj_tmp.dashStyle = GP.ClsDashStyles[GP.selectedClsIdx];
					GP.obj_tmp.penWidth = GP.ClsPenWidths[GP.selectedClsIdx];

                    // オブジェクト情報を登録
                    GP.objs.push_back(GP.obj_tmp);
                    //InvalidateRect(hWnd, NULL, TRUE);
                }
            }

            // 再描画
			InvalidateRect(hWnd, NULL, TRUE);

            // 後始末
            // テンポラリ矩形の座標をリセット
            GP.obj_tmp.rect.X = 0;
            GP.obj_tmp.rect.Y = 0;
            GP.obj_tmp.rect.Width = 0;
            GP.obj_tmp.rect.Height = 0;
            GP.makeBox = false; // ドラッグ中フラグを下ろす

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
				GP.imgIdx = (GP.imgIdx + GP.imgPaths.size() - 1) % GP.imgPaths.size();
			}
			break;
		case 'D': // 次の画像
		case VK_RIGHT:
			if (GP.imgPaths.size() > 0) {
				GP.imgIdx = (GP.imgIdx + 1) % GP.imgPaths.size();
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

