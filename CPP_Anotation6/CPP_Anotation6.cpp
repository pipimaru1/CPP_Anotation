    // CPP_Anotation.cpp : アプリケーションのエントリ ポイントを定義します。

#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFunctions.h"
#include "CPP_Anotation6.h"

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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

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
        SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, 0, (int)GP.imgObjs.size()); // タイトルバーに画像のパスを表示

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
            std::wstring _folderpath;
            //_folderpath = GetFolderPath(hWnd);
            //_folderpath = GetFolderPathIF(hWnd, GP.labelFolderPath, L"読込ラベルフォルダを選択してください"); // フォルダ選択ダイアログを表示
            //_folderpath = GetFolderPathEx(hWnd, GP.labelFolderPath, L"読込ラベルフォルダを選択してください"); // フォルダ選択ダイアログを表示
            _folderpath = GetFolderPathIFR(hWnd, L"読込ラベルフォルダを選択してください"); // フォルダ選択ダイアログを表示

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
			// ファイル保存ダイアログを表示
            std::wstring _folderpath;
            //_folderpath = GetFolderPath(hWnd);
            _folderpath = GetFolderPathIF(hWnd, GP.labelFolderPath, L"書込ラベルフォルダを選択してください"); // フォルダ選択ダイアログを表示

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

                        _fileName1 = GetFileNameFromPath(GP.imgObjs[i].path);
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
                WM_PAINT_DrawTmpBox(graphics, GP.tmpLabel.rect, GP.width, GP.height);
           
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
            GP.tmpLabel.rect.X = float(pt.x) / float(GP.width);
            GP.tmpLabel.rect.Y = float(pt.y) / float(GP.height);
            GP.tmpLabel.rect.Width = 0;
            GP.tmpLabel.rect.Height = 0;
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
            GP.tmpLabel.rect.Width = float(pt.x) / float(GP.width) - GP.tmpLabel.rect.X;
            GP.tmpLabel.rect.Height = float(pt.y) / float(GP.height) - GP.tmpLabel.rect.Y;
            NormalizeRect(GP.tmpLabel.rect); // 矩形の座標を正規化

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
        GP.tmpLabel.rect.X = 0;
        GP.tmpLabel.rect.Y = 0;
        GP.tmpLabel.rect.Width = 0;
        GP.tmpLabel.rect.Height = 0;
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
            GP.tmpLabel.rect.Width = float(pt.x) / float(GP.width) - GP.tmpLabel.rect.X;
            GP.tmpLabel.rect.Height = float(pt.y) / float(GP.height) - GP.tmpLabel.rect.Y;

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
                auto& r = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect;
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
			//size_t _obj_size = GP.imgObjs[GP.imgIdx].objs.size();
			//if (_obj_size > 0)
   //         {
   //             GP.imgObjs[GP.imgIdx].mOverIdx = GetIdxMouseOnRectEdge(pt, GP.imgObjs[GP.imgIdx].objs, GP.edMode, GP.Overlap); // マウスカーソルの位置を取得
   //         }

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
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Y = float(pt.y) / float(GP.height);
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Height = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Y - GP.prevMouse.Y;
                break;
            case EditMode::Bottom:
                // 下辺をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Height = float(pt.y) / float(GP.height) - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Y;
                break;
            case EditMode::Left:
                // 左辺をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.X = float(pt.x) / float(GP.width);
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Width = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.X - GP.prevMouse.X;
                break;
            case EditMode::Right:
                // 右辺をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Width = float(pt.x) / float(GP.width) - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.X;
                break;
            case EditMode::LeftTop:
                // 左上をドラッグ。 直接、マウスの値を辺の位置に設定   
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.X = float(pt.x) / float(GP.width);
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Y = float(pt.y) / float(GP.height);
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Width = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.X - GP.prevMouse.X;
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Height = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Y - GP.prevMouse.Y;
                break;
            case EditMode::RightTop:
                // 右上をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Y = float(pt.y) / float(GP.height);
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Width = float(pt.x) / float(GP.width) - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.X;
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Height = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Y - GP.prevMouse.Y;
                break;
            case EditMode::LeftBottom:
                // 左下をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.X = float(pt.x) / float(GP.width);
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Height = float(pt.y) / float(GP.height) - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Y;
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Width = GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.X - GP.prevMouse.X;
                break;
            case EditMode::RightBottom:
                // 右下をドラッグ。 直接、マウスの値を辺の位置に設定
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Width = float(pt.x) / float(GP.width) - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.X;
                GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Height = float(pt.y) / float(GP.height) - GP.imgObjs[GP.imgIdx].objs[GP.activeIdx].rect.Y;
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
   //         size_t _obj_size = GP.imgObjs[GP.imgIdx].objs.size();
			//if (_obj_size > 0) //これを入れないとリリース版の時にエラーが出る。関数呼び出しの不整合ではないかとのこと。
   //         {
   //             GP.imgObjs[GP.imgIdx].mOverIdx = GetIdxMouseOnRectEdge(pt, GP.imgObjs[GP.imgIdx].objs, GP.edMode, GP.Overlap);
   //         }
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
    {		// swtch文で入力されたキーを判定、処理を分ける
        // swtch文が二重になっている(Windowsプログラミングでは定石)

        // Shift が押されているかどうかを調べる
        bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        int step = 1;
		// Ctrl が押されているかどうかを調べる
		bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

        // Shift＋矢印なら大きく移動、小なら通常移動
        step = shift ? 10 : step;
		// Ctrl＋矢印なら大きく移動、小なら通常移動
		step = ctrl ? 100 : step;

        switch (wParam)
        {
        case 'A': // 前の画像
        case VK_LEFT:
            if (GP.imgObjs.size() > 0) {
                SaveLabelsToFileSingle(hWnd, GP.imgIdx);
                GP.imgIdx = (GP.imgIdx + GP.imgObjs.size() - step) % GP.imgObjs.size();
                SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.imgIdx, (int)GP.imgObjs.size()); // タイトルバーに画像のパスを表示
            }
            break;
        case 'D': // 次の画像
        case VK_RIGHT:
            if (GP.imgObjs.size() > 0) {
                SaveLabelsToFileSingle(hWnd, GP.imgIdx);
                GP.imgIdx = (GP.imgIdx + step) % GP.imgObjs.size();
                SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.imgIdx, (int)GP.imgObjs.size()); // タイトルバーに画像のパスを表示
            }
            break;

        case 'v': //前の画像からコピー
        case 'V': //前の画像からコピー
        case 'c': //前の画像からコピー
        case 'C': //前の画像からコピー
			//if (ctrl){
				if (GP.imgIdx > 0){
					//コピー前に確認のメッセージを表示
					int _copyok = MessageBoxW(hWnd, L"前の画像からコピーしますか？", L"確認", MB_OKCANCEL);

					if (_copyok == IDOK){
						// コピー処理
						GP.imgObjs[GP.imgIdx].objs = GP.imgObjs[GP.imgIdx - 1].objs;
						InvalidateRect(hWnd, NULL, TRUE); // 再描画
					}
				}
			//}
            break;
        }
        InvalidateRect(hWnd, NULL, TRUE); // 再描画
    }
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
// LabelObjを描画する関数
void WM_PAINT_DrawLabels(
    Gdiplus::Graphics& graphics,
    const std::vector<LabelObj>& objs,
    int clientWidth,
    int clientHeight,
    Gdiplus::Font* font
)
{
    for (const auto& obj : objs)
    {
        // ペン幅はマウスオーバーで太く
        int penWidth = obj.penWidth + (obj.mOver ? 2 : 0);
        Gdiplus::Pen pen(obj.color, static_cast<REAL>(penWidth));
        pen.SetDashStyle(obj.dashStyle);

        // 矩形の座標をピクセル変換
        float x0 = obj.rect.X * clientWidth;
        float y0 = obj.rect.Y * clientHeight;
        float w = obj.rect.Width * clientWidth;
        float h = obj.rect.Height * clientHeight;

        // 矩形描画
        graphics.DrawRectangle(&pen, x0, y0, w, h);

        // ラベル文字描画
        Gdiplus::SolidBrush textBrush(obj.color);
        const std::wstring& text = obj.ClassName;

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
// ラベルのクラス名をポップアップメニューで表示する関数
// 関数定義（例えば DrawingHelpers.cpp などにまとめてもOK）
void ShowClassPopupMenu(HWND hWnd)
{
    const int _perColumn = 20;

    // ポップアップメニューの作成
    HMENU hPopup = CreatePopupMenu();
    if (!hPopup) return;

    // カーソル位置を取得（画面→クライアント座標は不要）
    POINT pt;
    GetCursorPos(&pt);

    // メニュー項目を追加
   // for (size_t i = 0; i < GP.ClsNames.size(); ++i)
   // {


   //     //ショートカットキーを追加
   //     std::wostringstream clsName;
   //     clsName << L"(&" << GP.ClsNames[i].c_str()[0] << L")" << GP.ClsNames[i];

   //     // 分類がたくさんあるときの処理
   //     UINT flags = MF_STRING;
   //     // 「i が itemsPerColumn の倍数」のときは
   //     // この項目から新しい列を始める
   //     if (i > 0 && (i % _perColumn) == 0) {
   //         flags |= MF_MENUBREAK;
   //     }
   //     AppendMenuW(hPopup, 
   //         //MF_STRING,
			//flags,
   //         IDM_PMENU_CLSNAME00 + static_cast<UINT>(i),
			//clsName.str().c_str());
   //         //GP.ClsNames[i].c_str());
   // }

    // 先頭で未使用アクセラレータを管理するセットを用意
    std::set<wchar_t> usedAccels;

    // メニュー項目を追加 
    for (size_t i = 0; i < GP.ClsNames.size(); ++i)
    {
        const std::wstring& name = GP.ClsNames[i];
        wchar_t base = name[0];          // 元の頭文字
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
        L"(ESC)CANCEL");

    // ウィンドウを前面にしてから表示
    SetForegroundWindow(hWnd);
    UINT cmd = TrackPopupMenuEx(
        hPopup,
        TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
        pt.x, pt.y,
        hWnd,
        NULL
    );
    DestroyMenu(hPopup);

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
    return;
}

///////////////////////////////////////////////////////////////////////
// ラベルのクラス名をポップアップメニューで表示する関数
// 関数定義（例えば DrawingHelpers.cpp などにまとめてもOK）
void ShowClassPopupMenu_for_Edit(HWND hWnd, int activeObjectIDX )
{
    const int _perColumn = 20;

    // ポップアップメニューの作成
    HMENU hPopup = CreatePopupMenu();
    if (!hPopup) return;

    // カーソル位置を取得（画面→クライアント座標は不要）
    POINT pt;
    GetCursorPos(&pt);

    // メニュー項目を追加
    //for (size_t i = 0; i < GP.ClsNames.size(); ++i)
    //{
    //    //ショートカットキーを追加
    //    std::wostringstream clsName;
    //    clsName << L"(&" << GP.ClsNames[i].c_str()[0] << L")" << GP.ClsNames[i];

    //    UINT flags = MF_STRING;
    //    // 「i が itemsPerColumn の倍数」のときは
    //    // この項目から新しい列を始める
    //    if (i > 0 && (i % _perColumn) == 0) {
    //        flags |= MF_MENUBREAK;
    //    }
    //    AppendMenuW(hPopup,
    //        flags,
    //        IDM_PMENU_CLSNAME00 + static_cast<UINT>(i),
    //        clsName.str().c_str());
    //        //GP.ClsNames[i].c_str());
    //}

        // 先頭で未使用アクセラレータを管理するセットを用意
    std::set<wchar_t> usedAccels;

    // メニュー項目を追加 
    for (size_t i = 0; i < GP.ClsNames.size(); ++i)
    {
        const std::wstring& name = GP.ClsNames[i];
        wchar_t base = name[0];          // 元の頭文字
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
        L"DELETE");

    AppendMenuW(hPopup, MF_STRING,
        IDM_PMENU_CLSNAME00 + static_cast<UINT>(GP.ClsNames.size()) + 1,
        L"CANCEL");

    // ウィンドウを前面にしてから表示
    SetForegroundWindow(hWnd);
    UINT cmd = TrackPopupMenuEx(
        hPopup,
        TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
        pt.x, pt.y,
        hWnd,
        NULL
    );
    DestroyMenu(hPopup);

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
    }
    else if (cmd == IDM_PMENU_CLSNAME00 + static_cast<UINT>(GP.ClsNames.size()) + 1) // CANCEL
        return;

    return;
}