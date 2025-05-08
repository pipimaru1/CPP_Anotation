// CPP_Anotation.cpp : アプリケーションのエントリ ポイントを定義します。

#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFunctions.h"
#include "CPP_Anotation5.h"

// 必要なライブラリ
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")


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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。
    //GdiplusStartupInput gdiplusStartupInput;
    //GdiplusStartup(&g_GdiToken, &gdiplusStartupInput, NULL);
    //GdiplusRAII gdi;

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
// 上級課題
// WM_PIANTでファイルの画像を毎回読み込んで描画するのは処理が重いので、
// 画像データを保持しておき、必要なときに描画するようにする
// ヒント 画像データを保持するためImgObjectクラスに画像データを保持するメンバ変数を追加する
// 画像フォルダ設定時に画像データを読み込むようにする
// コーディング済　

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

//タイトルバーに画像のパスを表示
void SetStringToTitlleBar(HWND hWnd, std::wstring _imgfolder, std::wstring _labelfolder)
{
    std::wstring title = L"Annotation Tool - " + _imgfolder + L" - " + _labelfolder;
    SetWindowText(hWnd, title.c_str());
    return;
}


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
		GP.imgObjs.clear(); // 画像ファイルのパスと矩形の配列
		GP.imgFolderPath = INIT_IMGFOLDER; // フォルダパスを指定

        LoadImageFiles(GP.imgFolderPath, GP.imgObjs); // フォルダ内の画像ファイルを取得
        SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath); // タイトルバーに画像のパスを表示

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
        case IDM_LOAD_LABELS:
		{
			// ファイルオープンダイアログを表示
			std::wstring _folderpath = GetFolderPath(hWnd);
			
            // フォルダ選択ダイアログを表示
            if (!_folderpath.empty()) 
            {
                GP.labelFolderPath = _folderpath; // フォルダパスを指定

                // フォルダが選択された場合、アノテーションデータを読み込み
                //LoadLabelFiles(GP.imgObjs, GP.labelFolderPath, L".txt", 1);
                LoadLabelFilesMP(GP.imgObjs, GP.labelFolderPath, L".txt", 1);

                //タイトルバーに編集中の画像とラベルのパスを表示
                SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath); // タイトルバーに画像のパスを表示
            }
		}
		break;
        case IDM_SAVE_LABELS:
		{
			// ファイル保存ダイアログを表示
			std::wstring _folderpath = GetFolderPath(hWnd);
			// フォルダ選択ダイアログを表示
			if (!_folderpath.empty()) {
                GP.labelFolderPath = _folderpath; // フォルダパスを指定
                //タイトルバーに編集中の画像とラベルのパスを表示
                SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath); // タイトルバーに画像のパスを表示

                // フォルダが選択された場合、アノテーションデータを保存
                for (size_t i = 0; i < GP.imgObjs.size(); i++)
				{
					// ファイル名
                    std::wstring _fileName1;
                    std::wstring _fileName2;

                    _fileName1= GetFileNameFromPath(GP.imgObjs[i].path);
					_fileName2 = _folderpath + L"\\" + _fileName1 + L".txt";

                    bool _ret = SaveLabelsToFile(_fileName2, GP.imgObjs[i].objs, 1);
					if (!_ret)
					{
						// 保存失敗
						MessageBox(hWnd, L"保存失敗", L"失敗", MB_OK);
					}
				}
			}
		}
        break;

        case IDM_LOAD_IMAGES:
        {
            GP.imgFolderPath = GetFolderPath(hWnd); // フォルダ選択ダイアログを表示
            if (!GP.imgFolderPath.empty()) {
				// 画像ファイルのパスと矩形の配列をクリア
                GP.imgObjs.clear();
                // フォルダが選択された場合、画像ファイルを取得
                LoadImageFilesMP(GP.imgFolderPath, GP.imgObjs); // フォルダ内の画像ファイルを取得
                SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath); // タイトルバーに画像のパスを表示

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

    //////////////////////////////////////////////////////////////////////////////////
    // 描画処理
    //////////////////////////////////////////////////////////////////////////////////
    case WM_PAINT:
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
                if (GP.imgObjs[GP.imgIdx].image)
                {
                    //スケーリングしながら描画
                    graphics.DrawImage(GP.imgObjs[GP.imgIdx].image.get(), 0, 0, GP.width, GP.height);
                    graphics.Flush();

                }


                // 矩形を描画
                for (int i = 0; i < GP.imgObjs[GP.imgIdx].objs.size(); i++)
                {
                    LabelObj& _obj = GP.imgObjs[GP.imgIdx].objs[i];
                    
                    // マウスオーバーのとき
                    int _penWidth = _obj.penWidth;
                    if (_obj.mOver > 0)
                        _penWidth += 2; // マウスオーバー時はペン幅を太くする

                    Pen pen(_obj.color, _penWidth);

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
            }
            // ドラッグ中の矩形を描画
            if (GP.makeBox)
            {
                Pen pen(Color(255, 0, 0), 2);
                float x = GP.anno_tmp.rect.X;
                float y = GP.anno_tmp.rect.Y;
                float w = GP.anno_tmp.rect.Width;
                float h = GP.anno_tmp.rect.Height;

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
            //delete image;
            //image = nullptr; // 念のためnullにする
        }
        //if (GP.isMouseMoving) // マウス移動中
        {
            GP.isMouseMoving = false; // フラグをリセット
            ///////////////////////
            //正確な矩形を描くための目印になる水平線・垂直線をマウスカーソルに合わせて描画
            // マウスカーソルの位置を取得        
            POINT pt;
            // マウス座標をクライアント座標で取得
            GetCursorPos(&pt);
            // クライアント座標に変換
            ScreenToClient(hWnd, &pt);

            //ウィンドウのクライアント領域のサイズを取得
            RECT rect;
            GetClientRect(hWnd, &rect);

            //線の色はXORの色
            HDC hdc = GetDC(hWnd);
            SetROP2(hdc, R2_XORPEN);
            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
            SelectObject(hdc, hPen);

            // 水平線を描画
            MoveToEx(hdc, 0, pt.y, NULL);
            LineTo(hdc, rect.right - rect.left, pt.y);
            //垂直線を描画
            MoveToEx(hdc, pt.x, 0, NULL);
            LineTo(hdc, pt.x, rect.bottom - rect.top);

            DeleteObject(hPen);
            ReleaseDC(hWnd, hdc);
            //break;
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
            GP.anno_tmp.rect.X = float(pt.x) / float(GP.width);
            GP.anno_tmp.rect.Y = float(pt.y) / float(GP.height);
            GP.anno_tmp.rect.Width = 0;
            GP.anno_tmp.rect.Height = 0;

            GP.makeBox = true; // ドラッグ中フラグを立てる
            //GP.rects.push_back(rect); // 矩形を追加
        }
        else
        { 
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);

            // マウスキャプチャ開始
            SetCapture(hWnd);
            // 前回マウス位置にセット
            GP.prevMouse = Gdiplus::PointF((FLOAT)pt.x, (FLOAT)pt.y);
            // ヒットテストして、アクティブ矩形とドラッグモードを決定
            GP.activeIdx = GP.imgObjs[GP.imgIdx].mOverIdx;

        }

    
        
        SetCapture(hWnd); // マウスキャプチャを取得 ウィンドウの外に出てもマウスイベントを受け取る
    }
	break;
	// マウスの移動中の処理
	case WM_MOUSEMOVE:
	{
        // マウスの位置を取得
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hWnd, &pt);
        
        if (GP.makeBox) 
        {
		    GP.anno_tmp.rect.Width = float(pt.x) / float(GP.width) - GP.anno_tmp.rect.X;
            GP.anno_tmp.rect.Height = float(pt.y) / float(GP.height) - GP.anno_tmp.rect.Y;

            // 再描画
			InvalidateRect(hWnd, NULL, TRUE);
		}
        else
        {   
			//GP.isMouseMoving = true; // マウス移動中フラグを立てる
            GP.imgObjs[GP.imgIdx].mOverIdx = GetIdxMouseOnRectEdge(pt, GP.imgObjs[GP.imgIdx].objs, GP.Overlap); // マウスカーソルの位置を取得

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
            GP.anno_tmp.rect.Width = float(pt.x) / float(GP.width) - GP.anno_tmp.rect.X;
            GP.anno_tmp.rect.Height = float(pt.y) / float(GP.height) - GP.anno_tmp.rect.Y;
            NormalizeRect(GP.anno_tmp.rect); // 矩形の座標を正規化

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
                    GP.anno_tmp.ClassName = GP.ClsNames[GP.selectedClsIdx];
					GP.anno_tmp.ClassNum = GP.selectedClsIdx;
					GP.anno_tmp.color = GP.ClsColors[GP.selectedClsIdx];
					GP.anno_tmp.dashStyle = GP.ClsDashStyles[GP.selectedClsIdx];
					GP.anno_tmp.penWidth = GP.ClsPenWidths[GP.selectedClsIdx];

                    // オブジェクト情報を登録
                    if(GP.imgObjs.size() >0 )
    					GP.imgObjs[GP.imgIdx].objs.push_back(GP.anno_tmp);
                }
            }

            // 再描画
			InvalidateRect(hWnd, NULL, TRUE);

            // 後始末
            // テンポラリ矩形の座標をリセット
            GP.anno_tmp.rect.X = 0;
            GP.anno_tmp.rect.Y = 0;
            GP.anno_tmp.rect.Width = 0;
            GP.anno_tmp.rect.Height = 0;
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
            if (GP.imgObjs.size() > 0) {
				GP.imgIdx = (GP.imgIdx + GP.imgObjs.size() - 1) % GP.imgObjs.size();
			}
			break;
		case 'D': // 次の画像
		case VK_RIGHT:
			if (GP.imgObjs.size() > 0) {
				GP.imgIdx = (GP.imgIdx + 1) % GP.imgObjs.size();
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


