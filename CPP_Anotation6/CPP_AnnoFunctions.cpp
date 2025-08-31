#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFunctions.h"
#include "CPP_Anotation6.h"

#include "CPP_YoloAuto.h"

#pragma comment(lib, "Pathcch.lib")
#pragma comment(lib,"winmm.lib")

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
int GetImgsPaths(const std::wstring& folderPath, std::vector <ImgObject>& _imgObjs)
{
    _imgObjs.clear();
    _imgObjs.reserve(100);

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
            if (IsImageFile(fileName)) 
            {
				_imgObjs.emplace_back();//要素を追加
				ImgObject& _imgobj = _imgObjs.back(); // 追加した要素を参照
                
                _imgobj.path = folderPath;

                if (!_imgobj.path.empty() && _imgobj.path.back() != L'\\')
                    _imgobj.path += L'\\';
                _imgobj.path += fileName;

                _imgobj.image = std::make_unique<Gdiplus::Image>(_imgobj.path.c_str());
                // 失敗判定はポインタの null ではなく GDI+ ステータスで
                if (_imgobj.image->GetLastStatus() != Gdiplus::Ok) {
                    _imgobj.image = std::make_unique<Gdiplus::Image>(L"NO Image");
                }

                // 矩形の初期化
				_imgobj.objs.clear();
				_imgobj.objIdx = 0;
            }
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    return static_cast<int>(_imgObjs.size());
}
///////////////////////////////////////////////////////////////////////
// フォルダの画像ファイルから画像データを取得する関数
int LoadImageFiles(const std::wstring& folderPath, std::vector<ImgObject>& _imgObjs)
{
    _imgObjs.clear();
    _imgObjs.reserve(100);

    // ベースパスを先に作っておく
    std::wstring base = folderPath;
    if (!base.empty() && base.back() != L'\\') base += L'\\';
    std::wstring searchPath = base + L"*.*";

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    do {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            std::wstring fn = findData.cFileName;
            if (!IsImageFile(fn)) continue;

            // ① 要素を追加して参照を取得
			//ImgObject& img = _imgObjs.emplace_back(); //releaseだとエラーになる
            // Replace the problematic line with the following code:
            _imgObjs.emplace_back(); // Add a new element
            ImgObject& img = _imgObjs.back(); // Get a reference to the newly added element
            // ② パス設定
            img.path = base + fn;

            // ③ 画像読み込み（unique_ptr で安全管理）
			// メモリリーク防止のため、unique_ptr を使用
            img.image = std::make_unique<Gdiplus::Image>(img.path.c_str());

            // 失敗判定はポインタの null ではなく GDI+ ステータスで行う
            if (img.image->GetLastStatus() != Gdiplus::Ok) 
            {
				// 画像読み込み失敗時は NO Image を表示
                img.image = std::make_unique<Gdiplus::Image>(L"NO Image");
            }

            // ④ objs はデフォルトで空なので clear 不要、objIdx も既に 0
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    return static_cast<int>(_imgObjs.size());
}
///////////////////////////////////////////////////////////////////////
// フォルダの画像ファイルから画像データを取得する関数
int LoadImageFilesMP(const std::wstring& folderPath, std::vector<ImgObject>& _imgObjs)
{
    _imgObjs.clear();

    // 1) フォルダ中のファイル名を列挙してパスだけを貯める
    std::wstring base = folderPath;
    if (!base.empty() && base.back() != L'\\') base += L'\\';
    std::wstring searchPath = base + L"*.*";

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    std::vector<std::wstring> fileList;
    do {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            std::wstring fn = findData.cFileName;
            if (!IsImageFile(fn)) continue;
            fileList.push_back(base + fn);
        }
    } while (FindNextFileW(hFind, &findData));
    FindClose(hFind);

    const int N = static_cast<int>(fileList.size());
    if (N != 0)
    {
        // 2) 結果コンテナをあらかじめ確保しておく
        _imgObjs.resize(N);

        // 3) 画像ロードを並列化（チャンク単位）
        //const int CHUNK_SIZE = 8;
//#pragma omp parallel for schedule(static, CHUNK_SIZE)
#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < N; ++i) {
            ImgObject& img = _imgObjs[i];
            img.path = fileList[i];

#define _RELEASE_IMAGE
#ifdef RELEASE_IMAGE
            auto imgFile = std::make_unique<Gdiplus::Image>(img.path.c_str());
            if (imgFile->GetLastStatus() == Gdiplus::Ok)
            {
                auto w = imgFile->GetWidth();
                auto h = imgFile->GetHeight();
                auto pf = imgFile->GetPixelFormat();

                auto bmp = std::make_unique<Gdiplus::Bitmap>(w, h, pf);
                Gdiplus::Graphics g(bmp.get());
                g.DrawImage(imgFile.get(), 0, 0, w, h);

                imgFile.reset();              // ここでハンドル解放
                img.image = std::move(bmp); // 以降はメモリ上の Bitmap を保持
            }
#else
            // unique_ptr で管理
            auto image = std::make_unique<Gdiplus::Image>(img.path.c_str());
            if (image->GetLastStatus() != Gdiplus::Ok) {
                // ロード失敗時は代替イメージ
                image = std::make_unique<Gdiplus::Image>(L"NO Image");
            }
            img.image = std::move(image);
#endif
        }
    }
    else
        return 0;
    return N;
}

///////////////////////////////////////////////////////////////////////
// 矩形の座標を正規化する関数　Gdiplus::RectF
// X, Y：矩形の左上隅の座標,
// Width, Height：矩形の幅と高さ
void NormalizeRect(Gdiplus::RectF& r) 
{
    if (r.Width < 0) 
    {
        r.X += r.Width;
        r.Width = -r.Width;
    }
    if (r.Height < 0) 
    {
        r.Y += r.Height;
        r.Height = -r.Height;
    }

    if (r.X < 0)
        r.X = 0;
    if (r.Y < 0)
        r.Y = 0;
    if ((r.X + r.Width) > 1)
        r.Width = r.Width - ((r.X + r.Width) - 1);
    if ((r.Y + r.Height) > 1)
        r.Height = r.Height - ((r.Y + r.Height) - 1);
}

///////////////////////////////////////////////////////////////////////
// 矩形の座標をスケーリングする
void SscalingRect(Gdiplus::RectF& r_in, Gdiplus::RectF& r_out, float scaleX, float scaleY)
{
    // 元矩形の中心
    float cx = r_in.X + r_in.Width * 0.5f;
    float cy = r_in.Y + r_in.Height * 0.5f;

    // 新しい幅・高さ
    float newW = r_in.Width * scaleX;
    float newH = r_in.Height * scaleY;

    // 中心を固定して左上を求める
    r_out.X = cx - newW * 0.5f;
    r_out.Y = cy - newH * 0.5f;
    r_out.Width = newW;
    r_out.Height = newH;

    NormalizeRect(r_out);
}

///////////////////////////////////////////////////////////////////////
// フォルダ選択ダイアログを表示する関数
std::wstring GetFolderPath(HWND hWnd)
{
    std::wstring folderPath;
    IFileDialog* pFileDialog = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFileDialog));

    if (SUCCEEDED(hr)) {
        DWORD dwOptions = 0;
        pFileDialog->GetOptions(&dwOptions);
        pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);

        hr = pFileDialog->Show(hWnd);
        if (SUCCEEDED(hr)) {
            IShellItem* pItem = nullptr;
            hr = pFileDialog->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszFilePath = nullptr;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                if (SUCCEEDED(hr)) {
                    folderPath = pszFilePath;
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileDialog->Release();
    }
    return folderPath;
}
///////////////////////////////////////////////////////////////////////
// フォルダ選択ダイアログを表示する関数
// コールバック関数：ダイアログ起動時に初期フォルダを設定
static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED)
    {
        // lpData に渡した初期フォルダ文字列を使って選択を設定
        SendMessage(hwnd, BFFM_SETSELECTIONW, TRUE, lpData);
    }
    return 0;
}

// フォルダ選択ダイアログ（SHBrowseForFolder版）
// hWnd           : 親ウィンドウハンドル
// _currentFolder : 初期表示フォルダのパス（空文字なら既定）
// _title         : ダイアログのタイトル（空文字なら既定メッセージ）
std::wstring GetFolderPathEx(
    HWND hWnd,
    //const std::wstring& _currentFolder,
    const std::wstring& _title)
{
    wchar_t szPath[MAX_PATH] = { 0 };
    std::wstring _currentFolder;
	GetFolderPathfromReg(REGSTRY_KEYSTRING_FOLDER, _title, _currentFolder);


    BROWSEINFO bi = {};
    bi.hwndOwner = hWnd;
    bi.pszDisplayName = szPath;                  // 選択結果のバッファ
    bi.lpszTitle = _title.c_str();          // タイトルを指定
    bi.ulFlags = BIF_RETURNONLYFSDIRS     // フォルダのみを返す
        | BIF_USENEWUI            // 新しい UI
        | BIF_BROWSEINCLUDEFILES; // ファイルもツリーに表示
    bi.lpfn = BrowseCallbackProc;      // 初期フォルダ設定用コールバック
    bi.lParam = reinterpret_cast<LPARAM>(
        _currentFolder.empty()
        ? nullptr
        : _currentFolder.c_str());

    PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);
    if (pidl)
    {
        if (SHGetPathFromIDList(pidl, szPath))
        {
            CoTaskMemFree(pidl);
			SaveFolderPathToReg(REGSTRY_KEYSTRING_FOLDER, szPath, szPath);
            return std::wstring(szPath);
        }
        CoTaskMemFree(pidl);
    }
    return L"";  // キャンセル時など
}

///////////////////////////////////////////////////////////////////////
// フォルダ選択ダイアログ（IFileDialog版）
// hWnd: 親ウィンドウ
// _title: ダイアログ上部に表示するタイトル（空文字なら既定タイトル）
//レジストリに保存する
std::wstring GetFolderPathIFR(
    HWND hWnd,
    const std::wstring& dlgTitle,
    const std::wstring& regValueName // 空なら dlgTitle を値名に
)
{
    std::wstring result;
    //const wchar_t* subKey = REGSTRY_KEYSTRING_FOLDER;
    std::wstring valueName = regValueName.empty() ? dlgTitle : regValueName;

    // 1) レジストリから前回のフォルダを読み出し
    std::wstring initialFolder;
    GetFolderPathfromReg(REGSTRY_KEYSTRING_FOLDER, valueName, initialFolder);

    // 2) COM 初期化 → IFileDialog 作成
    HRESULT hr = CoInitializeEx(nullptr,
        COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) return L"";

    IFileDialog* pfd = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr) && pfd)
    {
        // フォルダ選択モードに
        DWORD opts = 0;
        pfd->GetOptions(&opts);
        pfd->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

        // 初期フォルダ
        if (!initialFolder.empty())
        {
            IShellItem* psiInit = nullptr;
            if (SUCCEEDED(SHCreateItemFromParsingName(
                initialFolder.c_str(),
                nullptr,
                IID_PPV_ARGS(&psiInit))))
            {
                pfd->SetFolder(psiInit);
                psiInit->Release();
            }
        }

        // タイトル設定
        if (!dlgTitle.empty())
            pfd->SetTitle(dlgTitle.c_str());

        // ダイアログ表示
        if (SUCCEEDED(pfd->Show(hWnd)))
        {
            IShellItem* psiRes = nullptr;
            if (SUCCEEDED(pfd->GetResult(&psiRes)))
            {
                PWSTR pszPath = nullptr;
                if (SUCCEEDED(psiRes->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)))
                {
                    result = pszPath;
                    CoTaskMemFree(pszPath);

                    // 3) 結果をレジストリに保存
					SaveFolderPathToReg(REGSTRY_KEYSTRING_FOLDER, valueName.c_str(),result.c_str());
                }
                psiRes->Release();
            }
        }

        pfd->Release();
    }
    CoUninitialize();
    return result;
}

///////////////////////////////////////////////////////////////////////
// フォルダーパスをレジストリから取得する関数
int GetFolderPathfromReg(
    const std::wstring& _subKey,
    const std::wstring& _regValueName,
    std::wstring& _folderPath
)
{
	int _ret = 0;
    //std::wstring  folderPath;
	HKEY hKey = nullptr;
	// レジストリキーを作成またはオープン
	if (RegCreateKeyExW(HKEY_CURRENT_USER, _subKey.c_str(), 0, nullptr,
		REG_OPTION_NON_VOLATILE, KEY_READ, nullptr,
		&hKey, nullptr) == ERROR_SUCCESS)
	{
		// フォルダパスをレジストリから取得
		wchar_t buf[MAX_PATH] = { 0 };
		DWORD bufSize = sizeof(buf), type = REG_SZ;
		if (RegQueryValueExW(hKey,
			_regValueName.c_str(),
			nullptr,
			&type,
			reinterpret_cast<BYTE*>(buf),
			&bufSize) == ERROR_SUCCESS)
		{
            _folderPath = buf;
			_ret = 1; // 成功
		}
        else
            _ret = 0;
		RegCloseKey(hKey);
	}
    else
        _ret = 0;
    return _ret;
}

///////////////////////////////////////////////////////////////////////
//フォルダーパスをレジストリに保存する関数
void SaveFolderPathToReg(
    const std::wstring _subKey,
    const std::wstring _regValueName,
    const std::wstring _folderPath
){
	HKEY hKey = nullptr;
	// レジストリキーを作成またはオープン
	if (RegCreateKeyExW(HKEY_CURRENT_USER, _subKey.c_str(), 0, nullptr,
		REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr,
		&hKey, nullptr) == ERROR_SUCCESS)
	{
		// フォルダパスをレジストリに保存
		RegSetValueExW(hKey, _regValueName.c_str(), 0, REG_SZ,
			reinterpret_cast<const BYTE*>(_folderPath.c_str()), 
			static_cast<DWORD>((wcslen(_folderPath.c_str()) + 1) * sizeof(wchar_t)));

			//static_cast<DWORD>((folderPath.size() + 1) * sizeof(wchar_t)));
		RegCloseKey(hKey);
	}
}

///////////////////////////////////////////////////////////////////////
// LabelObjをファイル保存するための文字列生成関数
// 入力値はLabelObj
// 出力値は文字列 std::wstring
// UTF-8で保存する
// YOLO形式で保存する
// wchar_t → UTF-8 の変換（C++17 の場合）
std::string LabelsToString(
    const LabelObj& obj, 
	int mode, // 0:default, 1:Yolo
	int _sc, // 0:正規化、1:スケールされた矩形を使用
	float minimumsize // 最小サイズ制限（デフォルトはなし）
)
{
    std::ostringstream oss;
    Gdiplus::RectF _rct;
    if (_sc != 0 ) 
        _rct = obj.Rct_Scale; // スケールされた矩形を使用
    else
		_rct = obj.Rct; // 正規化された矩形を使用

	if (minimumsize > 0){
        if (GP.isMinimumLabelCrrect){ //最小サイズ補正
            if (obj.Rct.Width < minimumsize)
            {
				_rct.X = obj.Rct.X - (minimumsize - obj.Rct.Width) / 2; // 中央寄せ
                _rct.Width = minimumsize; // 最小幅制限
            }
            if (obj.Rct.Height < minimumsize)
            {
				_rct.Y = obj.Rct.Y - (minimumsize - obj.Rct.Height) / 2; // 中央寄せ
                _rct.Height = minimumsize; // 最小高さ制限
            }
        }else{
            // 最小サイズ制限を適用 正規化された矩形で判定
            if (obj.Rct.Width < minimumsize || obj.Rct.Height < minimumsize){
                return ""; // サイズが小さすぎる場合は空文字列を返しておしまい
            }
        }
	}

    // 数値とスペースだけなので、普通の narrow string で OK
    if (mode == 0)
    {
        oss << obj.ClassNum << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.X) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.Y) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.Width) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.Height);
    }
	else if (mode == 1)
	{
        oss << obj.ClassNum << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.X + _rct.Width /2) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.Y + _rct.Height/2) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.Width) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.Height);
	}
	else
	{   
		// エラー処理
		return "";
	}
    return oss.str();  // 返り値の std::string は UTF-8 エンコード済み
}

///////////////////////////////////////////////////////////////////////
// LabelObjの文字列をファイル保存する関数
// 入力値はファイル名とconst std::vector<LabelObj>&
// 出力値は成功したらtrue、失敗したらfalse
bool SaveLabelsToFile(
    const std::wstring& fileName, 
    const std::vector<LabelObj>& objs, 
    int _sc,
	float minimumsize, // 最小サイズ制限（デフォルトはなし）
	int mode // 0:default, 1:yolo
){
	// UTF-8で保存するための設定
	std::ofstream file(fileName, std::ios::binary);

	// ファイルオープン
	if (!file.is_open()) {
		return false; // ファイルオープン失敗
	}
	for (const auto& obj : objs) {
		std::string _sc_str = LabelsToString(obj, mode, _sc, minimumsize);
		if (_sc_str.empty()) 
            continue; // 最小サイズ制限に引っかかった場合はスキップ
        else
			file << _sc_str << std::endl; // スケールされた矩形を使用
		//file << LabelsToString(obj, mode, minimumsize, _sc) << std::endl;
	}
	file.close();
	return true;
}

///////////////////////////////////////////////////////////////////////
// LabelObjの文字列をファイル保存する関数
// 入力値はファイル名とconst std::vector<LabelObj>&
// 出力値は成功したらtrue、失敗したらfalse
//bool SaveLabelsToFileWithScale(
//    const std::wstring& fileName,
//    const std::vector<LabelObj>& objs,
//	float scaleX = 1.0f, // X軸のスケール
//	float scaleY = 1.0f, // Y軸のスケール
//    int mode = 0 // 0:default, 1:yolo
//) {
//    // UTF-8で保存するための設定
//    std::ofstream file(fileName, std::ios::binary);
//
//    // ファイルオープン
//    if (!file.is_open()) {
//        return false; // ファイルオープン失敗
//    }
//    for (const auto& obj : objs) {
//        file << LabelsToString(obj, mode, true) << std::endl;
//    }
//    file.close();
//    return true;
//}
//

///////////////////////////////////////////////////////////////////////
std::wstring get_now_time_string()
{
    // 現在時刻を SYSTEMTIME で取得
    SYSTEMTIME st;
    GetLocalTime(&st);

    // std::tm に変換
    std::tm tm{};
    tm.tm_year = st.wYear - 1900;  // 年は 1900 起点
    tm.tm_mon = st.wMonth - 1;     // 月は 0-11
    tm.tm_mday = st.wDay;
    tm.tm_hour = st.wHour;
    tm.tm_min = st.wMinute;
    tm.tm_sec = st.wSecond;
    tm.tm_isdst = -1; // 夏時間の自動判定
    // std::mktime でタイムスタンプに変換
    //std::time_t now = std::mktime(&tm);
    // std::put_time を使ってフォーマット
    std::wostringstream oss;
    oss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S");
    return oss.str(); // std::wstring で返す
}

///////////////////////////////////////////////////////////////////////
// LabelObjの文字列をファイル保存する関数
// 画像ページングの時に使う
bool SaveLabelsToFileSingle(HWND hWnd, size_t _idx, float minimumsize) // 最小サイズ制限（デフォルトはなし）
{
    if (GP.imgObjs[GP.imgIdx].isEdited) {
        // ファイル名
        std::wstring _fileName1;
        std::wstring _fileName2;
        _fileName1 = GetOnlyFileNameFormPath(GP.imgObjs[GP.imgIdx].path);
        _fileName2 = GP.labelFolderPath + L"\\" + _fileName1 + L".txt";
        bool _ret = SaveLabelsToFile(_fileName2, GP.imgObjs[GP.imgIdx].objs, 0, minimumsize, 1);

        if (_ret) {
            //編集フラグをリセット
            GP.imgObjs[GP.imgIdx].isEdited = false;
            //MessageBeep(0x010L);  // MB_OKは「ポン」という標準音（クリック音に近い）
            PlaySound(SOUND_ARRY[15][1], NULL, SND_ALIAS | SND_ASYNC | SND_NODEFAULT); // 鳴らす  

            //書き込んだファイル名を"records.log"に書き込む
            //日時、ファイル名、ラベルの数
            std::wstring logFile = GP.labelFolderPath + L"\\records.log";
            std::wofstream logStream(logFile, std::ios::app);
            if (logStream.is_open()) {
                SYSTEMTIME st;
                GetLocalTime(&st);
                logStream << get_now_time_string() << L" "
                    << L"ANNOTATIONS:" << GP.imgObjs[GP.imgIdx].objs.size() << L" "
                    << GP.labelFolderPath + L"\\" + _fileName1 << std::endl;
                logStream.close();
            }

            return true;
        }
        else {
            // 保存失敗
            MessageBox(hWnd, L"保存失敗", L"失敗", MB_OK);
            return false;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////
// ファイル名をダイアログボックスで取得する関数
// 入力値はウィンドウハンドル
// 出力値はファイル名 std::wstring
std::wstring GetFileName_old(HWND hWnd)
{
	std::wstring fileName;
	IFileDialog* pFileDialog = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pFileDialog));
	if (SUCCEEDED(hr)) {
		// フィルタを設定
		COMDLG_FILTERSPEC filter[] = {
			{ L"YOLO", L"*.txt" },
			{ L"All Files", L"*.*" }
		};
		pFileDialog->SetFileTypes(ARRAYSIZE(filter), filter);
		// ダイアログを表示
		hr = pFileDialog->Show(hWnd);
		if (SUCCEEDED(hr)) {
			IShellItem* pItem = nullptr;
			hr = pFileDialog->GetResult(&pItem);
			if (SUCCEEDED(hr)) {
				PWSTR pszFilePath = nullptr;
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
				if (SUCCEEDED(hr)) {
					fileName = pszFilePath;
					CoTaskMemFree(pszFilePath);
				}
				pItem->Release();
			}
		}
		pFileDialog->Release();
	}
	return fileName;
}

///////////////////////////////////////////////////////////////////////
// ファイル名をダイアログボックスで取得する関数
// 入力値はウィンドウハンドル
// 出力値はファイル名 std::wstring
// ファイル選択ダイアログ＋レジストリ保存
std::wstring GetFileName(HWND hWnd, 
    const std::wstring& title, 
    COMDLG_FILTERSPEC filter[], 
    size_t filtersize, 
    int _rw)
{
    std::wstring fileName;

    // COMダイアログ作成
    IFileDialog* pFileDialog = nullptr;

    // ダイアログ種別を切り替え
    const CLSID clsid = (_rw == 1)
        ? CLSID_FileSaveDialog
        : CLSID_FileOpenDialog;


    HRESULT hr = CoCreateInstance(
        clsid,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFileDialog)
    );
    if (FAILED(hr)) return L"";

    // タイトル設定
    pFileDialog->SetTitle(title.c_str());

    // 前回のフォルダをレジストリから読み出し
    {
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, REGSTRY_KEYSTRING_FILE, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            wchar_t lastPath[MAX_PATH] = {};
            DWORD cb = sizeof(lastPath);
            if (RegQueryValueExW(hKey, title.c_str(), nullptr, nullptr, (BYTE*)lastPath, &cb) == ERROR_SUCCESS) {
                IShellItem* pFolder = nullptr;
                if (SUCCEEDED(SHCreateItemFromParsingName(lastPath, nullptr, IID_PPV_ARGS(&pFolder)))) {
                    pFileDialog->SetFolder(pFolder);
                    pFolder->Release();
                }
            }
            RegCloseKey(hKey);
        }
    }

    // フィルタ設定
    //COMDLG_FILTERSPEC filter[] = 
    //{
    //    { L"クラシフィケーションファイル",      L"*.txt" },
    //    { L"プリアノテーションファイル",        L"*.onnx" },
    //    { L"All Files", L"*.*"   }
    //};

    //pFileDialog->SetFileTypes(ARRAYSIZE(filter), filter);
    pFileDialog->SetFileTypes(filtersize, filter);

    // ダイアログ表示
    hr = pFileDialog->Show(hWnd);
    if (SUCCEEDED(hr)) {
        IShellItem* pItem = nullptr;
        hr = pFileDialog->GetResult(&pItem);
        if (SUCCEEDED(hr)) {
            PWSTR pszFilePath = nullptr;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
            if (SUCCEEDED(hr) && pszFilePath) {
                fileName = pszFilePath;
                CoTaskMemFree(pszFilePath);
            }
            pItem->Release();
        }

        // 選択されたファイルのフォルダ部分を抽出
        if (!fileName.empty()) {
            std::wstring folder = fileName;
            // PathCchRemoveFileSpec で末尾のファイル名を除去
            PathCchRemoveFileSpec(&folder[0], folder.size());
            // レジストリに保存（キーが無ければ作成）
            HKEY hKey = nullptr;
            if (RegCreateKeyExW(
                HKEY_CURRENT_USER,
                REGSTRY_KEYSTRING_FILE,
                0, nullptr,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                nullptr,
                &hKey,
                nullptr) == ERROR_SUCCESS)
            {
                RegSetValueExW(
                    hKey,
                    title.c_str(),
                    0, REG_SZ,
                    (const BYTE*)folder.c_str(),
                    static_cast<DWORD>((folder.size() + 1) * sizeof(wchar_t))
                );
                RegCloseKey(hKey);
            }
        }
    }

    pFileDialog->Release();
    return fileName;
}

///////////////////////////////////////////////////////////////////////
// ファイルパスから拡張子を指定の拡張子に変更する関数
// 入力値はファイルパスと拡張子
// 出力値は変更後のファイルパス std::wstring
std::wstring ChangeFileExtension(const std::wstring& filePath, const std::wstring& newExt)
{
	std::wstring newFilePath = filePath;
	size_t pos = newFilePath.find_last_of(L'.');
	if (pos != std::wstring::npos) {
		newFilePath.erase(pos);
	}
	newFilePath += newExt;
	return newFilePath;
}
///////////////////////////////////////////////////////////////////////
// ファイルパスから拡張子を取り除く関数
// 入力値はファイルパス
// 出力値は拡張子を取り除いたファイルパス std::wstring
std::wstring RemoveFileExtension(const std::wstring& filePath)
{
	std::wstring newFilePath = filePath;
	size_t pos = newFilePath.find_last_of(L'.');
	if (pos != std::wstring::npos) {
		newFilePath.erase(pos);
	}
	return newFilePath;
}

///////////////////////////////////////////////////////////////////////
// ファイルパスからファイル名のみ抽出する関数 
// 拡張子も削除
// 入力値はファイルパス
// 出力値はファイル名 std::wstring
std::wstring GetOnlyFileNameFormPath(const std::wstring& filePath)
{
    std::wstring _fileName1;
    std::wstring _fileName2;
	size_t pos = filePath.find_last_of(L'\\');
	if (pos != std::wstring::npos) {
        _fileName1 = filePath.substr(pos + 1);
	}
	_fileName2 = RemoveFileExtension(_fileName1);

	return _fileName2;
}
///////////////////////////////////////////////////////////////////////
// ファイルパスからファイル名のみ抽出する関数 拡張子は残す
// 入力値はファイルパス
// 出力値はファイル名 std::wstring
std::wstring GetFileNameFormPath(const std::wstring& filePath)
{
    std::wstring _fileName1;
    size_t pos = filePath.find_last_of(L'\\');
    if (pos != std::wstring::npos) {
        _fileName1 = filePath.substr(pos + 1);
    }
    return _fileName1; // 拡張子も含むファイル名を返す
}



///////////////////////////////////////////////////////////////////////
//フォルダパスとファイル名と拡張子を指定して
//ファイルが存在すれば、そのファイルのフルパスを返す関数
//ファイルが存在しなければ、空文字""を返す
std::wstring ChkFileExistWithPathExt(
    const std::wstring& folderpath, 
    const std::wstring& filename,
	const std::wstring& ext) // ".txt"など。"."を含むこと
{
	std::wstring _fn = GetOnlyFileNameFormPath(filename);

	std::wstring fullPath = folderpath + L"\\" + _fn + ext;
	if (PathFileExistsW(fullPath.c_str())) 
    {
		return fullPath; // ファイルが存在する場合、フルパスを返す
	}
	return L""; // ファイルが存在しない場合、空文字を返す
}

///////////////////////////////////////////////////////////////////////
// 関数 LoadLabels_to_Objects(
// ImgObject& imgObj, //データを格納するImgObjectクラスの参照
// const std::wstring& folderpath, //アノテーションファイルのあるフォルダパス
// const std::wstring& ext //アノテーションファイルの拡張子
// )
int LoadLabels_to_Objects(
ImgObject& imgObj, //データを格納するImgObjectクラスの参照
const std::wstring& folderpath, //アノテーションファイルのあるフォルダパス
const std::wstring& ext, //アノテーションファイルの拡張子
int mode //0:default, 1:yolo
){
	//アノテーションファイルのフルパスを取得
	std::wstring _fileName = ChkFileExistWithPathExt(folderpath, imgObj.path, ext);
	if (_fileName.empty()) 
    {
		return 0; // ファイルが存在しない場合、0を返す
	}
	
    //アノテーションファイルを開く
	std::wifstream file(_fileName);
	if (!file.is_open()) 
    {
		return 0; // ファイルオープン失敗
	}

	//アノテーションデータを読み込む
	imgObj.objs.clear(); // 既存のデータをクリア
    if (mode == 0)
    {
        LabelObj obj;
        while (file >> obj.ClassNum >> obj.Rct.X >> obj.Rct.Y >> obj.Rct.Width >> obj.Rct.Height)
        {

            imgObj.objs.push_back(obj);
        }
    }
	else if (mode == 1)
	{
		float tmp_x, tmp_y, tmp_w, tmp_h;
        LabelObj obj;
		// YOLO形式のデータを読み込む
		// YOLO形式は、クラス番号、x_center、y_center、width、heightの順
		// x_center、y_center、width、heightは画像サイズで割った値
		// 画像サイズで割った値を元に戻すために、imgObj.widthとimgObj.heightを使用
		while (file >> obj.ClassNum >> tmp_x >> tmp_y >> tmp_w >> tmp_h)
		{
            obj.Rct.X = tmp_x - tmp_w / 2;
			obj.Rct.Y = tmp_y - tmp_h / 2;
			obj.Rct.Width = tmp_w;
            obj.Rct.Height = tmp_h;
			// YOLO形式のデータをLabelObj形式に変換
			imgObj.objs.push_back(obj);
		}
	}
	file.close();
	return static_cast<int>(imgObj.objs.size());
}

///////////////////////////////////////////////////////////////////////
// 関数 LoadLabelFiles(
// std::vector<ImgObject>& imgObjs, //データを格納するImgObjectクラスの参照
// const std::wstring& folderpath, //アノテーションファイルのあるフォルダパス
// const std::wstring& ext //アノテーションファイルの拡張子
// )
int LoadLabelFiles(
	std::vector<ImgObject>& imgObjs, //データを格納するImgObjectクラスの参照
	const std::wstring& folderpath, //アノテーションファイルのあるフォルダパス
	const std::wstring& ext, //アノテーションファイルの拡張子
    int mode //0:default, 1:yolo
){
	int loadCount = 0; // 読み込んだアノテーションの数
	for (int i = 0; i < imgObjs.size(); i++)
	{
        //アノテーションファイルのフルパスを取得
        std::wstring _fileName = ChkFileExistWithPathExt(folderpath, imgObjs[i].path, ext);
        if (_fileName.empty())
        {
			continue; // ファイルが存在しない場合、次の画像へ
        }
        //アノテーションファイルを開く
        std::wifstream file(_fileName);
        if (!file.is_open())
        {
            continue; // ファイルオープン失敗
        }
        imgObjs[i].objs.clear(); // 既存のデータをクリア
        
        
        if (mode == 0)
        {
            LabelObj obj;
            while (file >> obj.ClassNum >> obj.Rct.X >> obj.Rct.Y >> obj.Rct.Width >> obj.Rct.Height)
            {
				NormalizeRect(obj.Rct); // 矩形の座標を正規化

                if (obj.ClassNum < GP.ClsNames.size())
                    obj.ClassName = GP.ClsNames[obj.ClassNum];

                if (obj.ClassNum < GP.ClsColors.size())
                    obj.color = GP.ClsColors[obj.ClassNum];

                if (obj.ClassNum < GP.ClsDashStyles.size())
                    obj.dashStyle = GP.ClsDashStyles[obj.ClassNum];

                if (obj.ClassNum < GP.ClsPenWidths.size())
                    obj.penWidth = GP.ClsPenWidths[obj.ClassNum];

                imgObjs[i].objs.push_back(obj);
            }
        }
        else if (mode == 1)
        {
			float tmp_x, tmp_y, tmp_w, tmp_h;
			LabelObj obj;
			// YOLO形式のデータを読み込む
			// YOLO形式は、クラス番号、x_center、y_center、width、heightの順
			// x_center、y_center、width、heightは画像サイズで割った値
			// 画像サイズで割った値を元に戻すために、imgObj.widthとimgObj.heightを使用
			while (file >> obj.ClassNum >> tmp_x >> tmp_y >> tmp_w >> tmp_h)
			{
				obj.Rct.X = tmp_x - tmp_w / 2;
				obj.Rct.Y = tmp_y - tmp_h / 2;
				obj.Rct.Width = tmp_w;
				obj.Rct.Height = tmp_h;

                NormalizeRect(obj.Rct); // 矩形の座標を正規化
                
                if (obj.ClassNum < GP.ClsNames.size())
					obj.ClassName = GP.ClsNames[obj.ClassNum];
				if (obj.ClassNum < GP.ClsColors.size())
					obj.color = GP.ClsColors[obj.ClassNum];
				if (obj.ClassNum < GP.ClsDashStyles.size())
					obj.dashStyle = GP.ClsDashStyles[obj.ClassNum];
				if (obj.ClassNum < GP.ClsPenWidths.size())
					obj.penWidth = GP.ClsPenWidths[obj.ClassNum];
				imgObjs[i].objs.push_back(obj);
			}
        }
        else
			continue;// 何もしない

        file.close();
        loadCount++;
    }
    return loadCount;
}

int LoadLabelFilesMP(
    std::vector<ImgObject>& imgObjs,
    const std::wstring& folderpath,
    const std::wstring& ext,
    int mode
){
    int loadCount = 0;
    const int N = static_cast<int>(imgObjs.size());

    // OpenMP 並列化：各 i は独立処理なので競合なし
#pragma omp parallel for schedule(dynamic) reduction(+:loadCount)
    for (int i = 0; i < N; ++i)
    {
        // ファイル名取得（スレッドごとにローカルな変数）
        std::wstring _fileName = ChkFileExistWithPathExt(folderpath, imgObjs[i].path, ext);
        if (_fileName.empty()) continue;

        // ファイルを開いてパース
        std::wifstream file(_fileName);
        if (!file.is_open()) continue;

        // 既存データをクリア
        imgObjs[i].objs.clear();

        if (mode == 0)
        {
            LabelObj obj;
            while (file >> obj.ClassNum
                >> obj.Rct.X >> obj.Rct.Y
                >> obj.Rct.Width >> obj.Rct.Height)
            {
                NormalizeRect(obj.Rct);

                if (obj.ClassNum < GP.ClsNames.size())     obj.ClassName = GP.ClsNames[obj.ClassNum];
                if (obj.ClassNum < GP.ClsColors.size())    obj.color = GP.ClsColors[obj.ClassNum];
                if (obj.ClassNum < GP.ClsDashStyles.size())obj.dashStyle = GP.ClsDashStyles[obj.ClassNum];
                if (obj.ClassNum < GP.ClsPenWidths.size()) obj.penWidth = GP.ClsPenWidths[obj.ClassNum];

                imgObjs[i].objs.push_back(obj);
            }
        }
        else if (mode == 1)
        {
            float tmp_x, tmp_y, tmp_w, tmp_h;
            LabelObj obj;
            while (file >> obj.ClassNum >> tmp_x >> tmp_y >> tmp_w >> tmp_h)
            {
                // YOLO->BBox
                obj.Rct.X = tmp_x - tmp_w / 2;
                obj.Rct.Y = tmp_y - tmp_h / 2;
                obj.Rct.Width = tmp_w;
                obj.Rct.Height = tmp_h;
                NormalizeRect(obj.Rct);

                if (obj.ClassNum < GP.ClsNames.size())     obj.ClassName = GP.ClsNames[obj.ClassNum];
                if (obj.ClassNum < GP.ClsColors.size())    obj.color = GP.ClsColors[obj.ClassNum];
                if (obj.ClassNum < GP.ClsDashStyles.size())obj.dashStyle = GP.ClsDashStyles[obj.ClassNum];
                if (obj.ClassNum < GP.ClsPenWidths.size()) obj.penWidth = GP.ClsPenWidths[obj.ClassNum];

                imgObjs[i].objs.push_back(obj);
            }
        }
        // mode がそれ以外なら何もしない
        file.close();

        // この画像は成功読み込みとみなしてカウント
        loadCount++;
    }

    return loadCount;
}

///////////////////////////////////////////////////////////////////////
//矩形の線上にマウスカーソルがあるかどうかを判定する関数
EditMode IsMouseOnRectEdge(
    const POINT& pt, 
    const LabelObj& obj,
    int overlap
){
	bool Left = false;
	bool Right = false;
	bool Top = false;
	bool Bottom = false;

    // 矩形の座標
    float x0 = obj.Rct.X * GP.width;
    float y0 = obj.Rct.Y * GP.height;
    float w = obj.Rct.Width * GP.width;
    float h = obj.Rct.Height * GP.height;

    // 各辺の幅を帯状に見立てて判定
	EditMode  _ret = EditMode::None;
    if(pt.y >= y0 - overlap && pt.y <= y0 + overlap &&
        pt.x >= x0 - overlap && pt.x <= x0 + w + overlap)
		Top = true; // 上辺

    if(pt.y >= y0 + h - overlap && pt.y <= y0 + h + overlap &&
        pt.x >= x0 - overlap && pt.x <= x0 + w + overlap)
		Bottom = true; // 下辺

    if(pt.x >= x0 - overlap && pt.x <= x0 + overlap &&
        pt.y >= y0 - overlap && pt.y <= y0 + h + overlap)
		Left = true; // 左辺

    if(pt.x >= x0 + w - overlap && pt.x <= x0 + w + overlap &&
        pt.y >= y0 - overlap && pt.y <= y0 + h + overlap)
		Right = true; // 右辺

    if (Top && Left)
        return EditMode::LeftTop; // 上辺左端
	if (Bottom && Left)
		return EditMode::LeftBottom; // 下辺左端
	if (Top && Right)
		return EditMode::RightTop; // 上辺右端
	if (Bottom && Right)
		return EditMode::RightBottom; // 下辺右端
	if (Top)
		return EditMode::Top; // 上辺
	if (Bottom)
		return EditMode::Bottom; // 下辺
	if (Left)
		return EditMode::Left; // 左辺
	if (Right)
		return EditMode::Right; // 右辺

	return EditMode::None; // 外 
}
///////////////////////////////////////////////////////////////////////
//矩形の線上にマウスカーソルがあるかどうかを判定する関数
int IsMouseOnRectEdge_old(
    const POINT& pt,
    const LabelObj& obj,
    int overlap
){
    // 矩形の座標
    float x0 = obj.Rct.X * GP.width;
    float y0 = obj.Rct.Y * GP.height;
    float w = obj.Rct.Width * GP.width;
    float h = obj.Rct.Height * GP.height;

    // 各辺の幅を帯状に見立てて判定
    int _ret = 0;
    if (pt.y >= y0 - overlap && pt.y <= y0 + overlap &&
        pt.x >= x0 - overlap && pt.x <= x0 + w + overlap)
        return 1; // 上辺

    if (pt.y >= y0 + h - overlap && pt.y <= y0 + h + overlap &&
        pt.x >= x0 - overlap && pt.x <= x0 + w + overlap)
        return 2; // 下辺

    if (pt.x >= x0 - overlap && pt.x <= x0 + overlap &&
        pt.y >= y0 - overlap && pt.y <= y0 + h + overlap)
        return 3; // 左辺

    if (pt.x >= x0 + w - overlap && pt.x <= x0 + w + overlap &&
        pt.y >= y0 - overlap && pt.y <= y0 + h + overlap)
        return 4; // 右辺

    return 0; // 外
}

///////////////////////////////////////////////////////////////////////
//マウスカーソルと重なる矩形のインデックスを取得する関数
//最初の一つだけを返す
//重なった矩形のオブジェクトには_mOverに1～8の値が入る
size_t GetIdxMouseOnRectEdge(
    const POINT& pt,
    std::vector<LabelObj>& objs,
	EditMode& editMode,
    int overlap
){
    int _idx=-1;

	//全部の矩形のマウスオーバー状態を解除
    for (size_t i = 0; i < objs.size(); i++)
    //for (size_t i = 0; i < _obj_size; i++)
            objs[i].mOver = false;

    for (size_t i = 0; i < objs.size(); i++)
    {
        //辺の位置が返ってくる。
		EditMode _em = IsMouseOnRectEdge(pt, objs[i], overlap);
        if (_em !=  EditMode::None )
        {
            objs[i].mOver = true;
            editMode = _em; // 選択状態にする
            _idx = i; // 最初の一つだけを返す
            break;
        }
        else
        {
            editMode = EditMode::None; // 選択状態を解除
        }
    }
    return _idx; // 矩形がない場合は-1を返す
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
        float x0 = obj.Rct.X * clientWidth;
        float y0 = obj.Rct.Y * clientHeight;
        float w = obj.Rct.Width * clientWidth;
        float h = obj.Rct.Height * clientHeight;

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

int ShowClassPopupMenu_for_Edit(HWND hWnd, ImgObject& _imgobj, int activeObjectIDX)
{
    // 選択結果を反映
    UINT cmd = 0;
    if (ShowClassPopupMenu_Core(hWnd, cmd))
    {
        if (cmd >= IDM_PMENU_CLSNAME00 &&
            cmd < IDM_PMENU_CLSNAME00 + GP.ClsNames.size())
        {
            GP.selectedClsIdx = cmd - IDM_PMENU_CLSNAME00;

            // activeObjectIDXで示すオブジェクトに選択内容を上書き
            //GP.imgObjs[GP.imgIdx].objs[activeObjectIDX].ClassName = GP.ClsNames[GP.selectedClsIdx];
            //GP.imgObjs[GP.imgIdx].objs[activeObjectIDX].ClassNum = GP.selectedClsIdx;
            //GP.imgObjs[GP.imgIdx].objs[activeObjectIDX].color = GP.ClsColors[GP.selectedClsIdx];
            //GP.imgObjs[GP.imgIdx].objs[activeObjectIDX].dashStyle = GP.ClsDashStyles[GP.selectedClsIdx];
            //GP.imgObjs[GP.imgIdx].objs[activeObjectIDX].penWidth = GP.ClsPenWidths[GP.selectedClsIdx];

            _imgobj.objs[activeObjectIDX].ClassName = GP.ClsNames[GP.selectedClsIdx];
            _imgobj.objs[activeObjectIDX].ClassNum = GP.selectedClsIdx;
            _imgobj.objs[activeObjectIDX].color = GP.ClsColors[GP.selectedClsIdx];
            _imgobj.objs[activeObjectIDX].dashStyle = GP.ClsDashStyles[GP.selectedClsIdx];
            _imgobj.objs[activeObjectIDX].penWidth = GP.ClsPenWidths[GP.selectedClsIdx];

            GP.imgObjs[GP.imgIdx].isEdited = true; // 編集フラグを立てる

            return GP.selectedClsIdx;   // 選択されたクラス番号
        }
        else if (cmd == IDM_PMENU_CLSNAME00 + static_cast<UINT>(GP.ClsNames.size())) // DELETE
        {
            // オブジェクトを削除
            //GP.imgObjs[GP.imgIdx].objs.erase(GP.imgObjs[GP.imgIdx].objs.begin() + activeObjectIDX);
            _imgobj.objs.erase(_imgobj.objs.begin() + activeObjectIDX);
            _imgobj.isEdited = true; // 編集フラグを立てる このフラグは使われないが…

            return -1; // 削除したことを示す
        }
        else if (cmd == IDM_PMENU_CLSNAME00 + static_cast<UINT>(GP.ClsNames.size()) + 1) // CANCEL
        {
            return -2; // キャンセルしたことを示す
        }
    }
    else
    {
        return -3; // エラー
	}
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
    if (ShowClassPopupMenu_Core(hWnd, cmd))
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

////////////////////////////////////////////////////////////////////////
// ラベルのクラス名をポップアップメニューで表示する関数 
// ラップ
int ShowClassPopupMenu_for_Edit(HWND hWnd, int activeObjectIDX)
{
    return ShowClassPopupMenu_for_Edit(hWnd, GP.imgObjs[GP.imgIdx], activeObjectIDX);
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


// _startIdx: 検索開始の画像インデックス（既定は 0）
// minW/minH: 幅・高さの閾値（正規化済み or ピクセルに合わせて）
std::optional<size_t> jumpImgWithIgnoreBox(
    const std::vector<ImgObject>& imgObjs,
    size_t _startIdx,
    float minW,
    float minH)
{
    if (_startIdx >= imgObjs.size())
        return std::nullopt;

    for (size_t imgIdx = _startIdx; imgIdx < imgObjs.size(); ++imgIdx)
    {
        const auto& img = imgObjs[imgIdx];
        for (const auto& lb : img.objs)
        {
            // スケール後矩形 or ノーマル矩形を選択
            //const auto& rc = (lb.Rct_Scale.Width > 0 && lb.Rct_Scale.Height > 0)
            //    ? lb.Rct_Scale
            //    : lb.Rct;
            //const auto& rc = lb.Rct;
            //if (rc.Width <= minW || rc.Height <= minH)
            //    return imgIdx;
            if (isIgnoreBox(lb, minW, minH))
            {
                return imgIdx; // 最初に見つかった画像インデックスを返す
            }
        }
    }
    return std::nullopt;
}

bool isIgnoreBox(
    const LabelObj& obj,
    float minW,
    float minH)
{
    bool _ret = false;
    _ret = (obj.Rct.Width <= minW || obj.Rct.Height <= minH);
    return _ret;
}

/// @brief フォルダパスの末尾ディレクトリ名の直前に"deleted"フォルダを挿入する
/// @param folderPath 元のフォルダパス（例: L"C:\\hoge1\\hoge2\\labels"）
/// @return 挿入後のフォルダパス（例: L"C:\\hoge1\\hoge2\\deleted\\labels"）
std::wstring InsertSubFolder(const std::wstring& folderPath, const std::wstring& _sub)
{
    // 末尾の'\'または'/'を除去
    std::wstring path = folderPath;
    if (!path.empty() && (path.back() == L'\\' || path.back() == L'/')) {
        path.pop_back();
    }

    // 最後の区切り文字位置を取得
    size_t pos = path.find_last_of(L"\\/");
    std::wstring parent, leaf;
    if (pos == std::wstring::npos) {
        // 区切り文字なしなら全体をleafとみなす
        parent.clear();
        leaf = path;
    }
    else {
        parent = path.substr(0, pos);
        leaf = path.substr(pos + 1);
    }

    // 組み立て
    if (!parent.empty()) {
        return parent + L"\\" + _sub + L"\\" + leaf;
        //return parent + L"\\deleted\\" + leaf;
    }
    else {
        return _sub + L"\\" + leaf;
        //return L"deleted\\" + leaf;
    }
}

///////////////////////////////////////////////////////////////////////////////////
inline void FlushLeftMouseClick()
{
    // 1) 左ボタンが離されるまで待つ
    while (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        Sleep(10);
    }

    // 2) キュー内の残っている WM_LBUTTONDOWN ～ WM_LBUTTONUP を取り除く
    MSG msg;
    while (PeekMessage(&msg, nullptr,
        WM_LBUTTONDOWN, WM_LBUTTONUP,
        PM_REMOVE))
    {
        // 何もしない
    }
}

///////////////////////////////////////////////////////////////////////////////////
// 指定idxの画像とラベルを移動
int MoveCurrentImageAndLabel(HWND hWnd, int imgIdx)
{
    if (imgIdx < 0 || imgIdx >= static_cast<int>(GP.imgObjs.size())) {
        MessageBox(hWnd, L"無効な画像インデックスです。", L"エラー", MB_OK | MB_ICONERROR);
        return -1; // エラーコード
    }
    // 現在の画像とラベルを保存
    SaveLabelsToFileSingle(hWnd, GP.imgIdx, 0.0f);

    std::wstring _ImgFilePath = GP.imgObjs[GP.imgIdx].path;
    std::wstring _fn1 = GetOnlyFileNameFormPath(_ImgFilePath);
    std::wstring _LabelFilePath = GP.labelFolderPath + L"\\" + _fn1 + L".txt";

    // 移動先のフォルダパスを作成
    std::wstring _tempImagePath = InsertSubFolder(GP.imgFolderPath, L"deleted");
    std::wstring _tempLabelPath = InsertSubFolder(GP.labelFolderPath, L"deleted");

    // 移動先のファイルパスを作成
    std::wstring _tempImageFilePath = _tempImagePath + L"\\" + _fn1 + L".jpg";
    std::wstring _tempLabelFilePath = _tempLabelPath + L"\\" + _fn1 + L".txt";

    // 移動先のフォルダが存在しない場合は作成
    if (!PathFileExistsW(_tempImagePath.c_str()))
    {
        //if (!CreateDirectoryW(_tempImagePath.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        if (!std::filesystem::create_directories(_tempImagePath))
        {
            MessageBox(hWnd, _tempImagePath.c_str(), L"移動先のフォルダの作成に失敗しました", MB_OK | MB_ICONERROR);
            FlushLeftMouseClick();
            return -1; // エラーコード
        }
    }
    if (!PathFileExistsW(_tempLabelPath.c_str())) {
        //if (!CreateDirectoryW(_tempLabelPath.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
        if (!std::filesystem::create_directories(_tempLabelPath))
        {
            MessageBox(hWnd, _tempLabelPath.c_str(), L"移動先のフォルダの作成に失敗しました", MB_OK | MB_ICONERROR);
            FlushLeftMouseClick();
            return -1; // エラーコード
        }
    }

    // ★ここから存在チェックを追加
    auto FileExists = [](const std::wstring& path) -> bool {
        return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
        };

    if (!FileExists(_ImgFilePath)) {
        MessageBox(hWnd, (_ImgFilePath + L"\n(画像が見つかりません)").c_str(),
            L"移動失敗", MB_OK | MB_ICONERROR);
        FlushLeftMouseClick();
        return -1;
    }
#ifdef _DEBUG
    if (!FileExists(_LabelFilePath)) {
        MessageBox(hWnd, (_LabelFilePath + L"\n(ラベルが見つかりません 続行します)").c_str(),
            L"警告", MB_OK | MB_ICONWARNING);
        FlushLeftMouseClick();
    }
#endif   

#ifndef RELEASE_IMAGE
    // ① 画像ファイルのハンドルを解放してから移動する
    if (GP.imgIdx < GP.imgObjs.size() && GP.imgObjs[GP.imgIdx].image)
        GP.imgObjs[GP.imgIdx].image.reset();   // ← unique_ptr をリセット
#endif

    if (!MoveFileEx(_ImgFilePath.c_str(),
        _tempImageFilePath.c_str(),   // ★ファイル名付きパスを渡す
        MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING))
    {
        std::wstring _msg = _ImgFilePath + L"\n" + _tempImageFilePath;
        MessageBox(hWnd, _msg.c_str(), L"画像の移動に失敗しました", MB_OK | MB_ICONERROR);
        FlushLeftMouseClick();
        return -1;
    }

    // ラベル
    if (!MoveFileEx(_LabelFilePath.c_str(),
        _tempLabelFilePath.c_str(),   // ★こちらも同様
        MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING))
    {
        std::wstring _msg = _LabelFilePath + L"\n" + _tempLabelFilePath;
#ifdef _DEBUG
        MessageBox(hWnd, _msg.c_str(), L"ラベルファイルはなかったようです 続行します",
            MB_OK | MB_ICONWARNING);
        FlushLeftMouseClick();
#endif   
    }

    //メモリ上の画像とラベルを削除
    GP.imgObjs.erase(GP.imgObjs.begin() + GP.imgIdx); // 現在の画像とラベルを削除
    //GP.imgIdx = imgIdx;

    //マウスのクリックをフラッシュ
    //FlushLeftMouseClick();

    // タイトルバーに新しい画像とラベルのパスを表示
    SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.imgIdx, static_cast<int>(GP.imgObjs.size()));
    // 再描画
    InvalidateRect(hWnd, NULL, TRUE);

    FlushLeftMouseClick();
    return 0; // 成功
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

        // YOLOの推論結果を描画
        if (g_showProposals && !AutoDetctedObjs.objs.empty()) {
            // 1) 画像を既存ロジックで描画（例：Fit表示）
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            //const int imgW = GP.imgObjs[GP.imgIdx].image->GetWidth(); // ←あなたの構造体の実フィールド名に合わせてください
            //const int imgH = GP.imgObjs[GP.imgIdx].image->GetHeight(); // 例示
            //RectF view = FitImageToClientRect(imgW, imgH, rcClient);
            //DrawLabelObjects(graphics, AutoDetctedObjs, view);
            DrawLabelObjects(graphics, AutoDetctedObjs.objs, ToRectF(rcClient));
        }

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

void CheckMenues(HWND hWnd)
{
    CheckMenu(hWnd, IDM_YOLO_IMGSZE640, false);
    CheckMenu(hWnd, IDM_YOLO_IMGSZE1280, false);
    CheckMenu(hWnd, IDM_YOLO_IMGSZE1920, false);
    CheckMenu(hWnd, IDM_YOLO_SELCTONNX, false);
       

    if(GDNNP.yolo.inputW == 640)
        CheckMenu(hWnd, IDM_YOLO_IMGSZE640, true);
    if (GDNNP.yolo.inputW == 1280)
        CheckMenu(hWnd, IDM_YOLO_IMGSZE1280, true);
    if (GDNNP.yolo.inputW == 1920)
        CheckMenu(hWnd, IDM_YOLO_IMGSZE1920, true);

	if (!GDNNP.gOnnxPath.empty())
    {
        CheckMenu(hWnd, IDM_YOLO_SELCTONNX, true);
    }
}
