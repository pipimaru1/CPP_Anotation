#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFunctions.h"
#include "CPP_Anotation6.h"

#pragma comment(lib, "Pathcch.lib")


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

        // 3) 画像ロードを並列化
#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < N; ++i) {
            ImgObject& img = _imgObjs[i];
            img.path = fileList[i];

            // unique_ptr で管理
            auto image = std::make_unique<Gdiplus::Image>(img.path.c_str());
            if (image->GetLastStatus() != Gdiplus::Ok) {
                // ロード失敗時は代替イメージ
                image = std::make_unique<Gdiplus::Image>(L"NO Image");
            }
            img.image = std::move(image);
        }
    }
    else
        return 0;
    return N;
}

///////////////////////////////////////////////////////////////////////
// 矩形の座標を正規化する関数
void NormalizeRect(RectF& r) 
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
    const std::wstring& _currentFolder,
    const std::wstring& _title)
{
    wchar_t szPath[MAX_PATH] = { 0 };

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
            return std::wstring(szPath);
        }
        CoTaskMemFree(pidl);
    }
    return L"";  // キャンセル時など
}
///////////////////////////////////////////////////////////////////////
// フォルダ選択ダイアログ（IFileDialog版）
// hWnd: 親ウィンドウ
// _currentFolder: 初期表示フォルダのパス（空文字なら既定フォルダ）
// _title: ダイアログ上部に表示するタイトル（空文字なら既定タイトル）
std::wstring GetFolderPathIF(
    HWND hWnd,
    const std::wstring& _currentFolder,
    const std::wstring& _title)
{
    std::wstring result;

    // COM 初期化
    HRESULT hr = CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    IFileDialog* pfd = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr) && pfd)
    {
        // フォルダ選択モードにする
        DWORD dwOptions;
        pfd->GetOptions(&dwOptions);
        pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR);

        // 初期フォルダを設定
        if (!_currentFolder.empty()){
            IShellItem* psiInit = nullptr;
            if (SUCCEEDED(SHCreateItemFromParsingName(_currentFolder.c_str(),
                nullptr,IID_PPV_ARGS(&psiInit))))
            {
                pfd->SetFolder(psiInit);
                psiInit->Release();
            }
        }

        // タイトルを設定
        if (!_title.empty())
            pfd->SetTitle(_title.c_str());

        // ダイアログ表示
        if (SUCCEEDED(pfd->Show(hWnd))){
            IShellItem* psiResult = nullptr;
            if (SUCCEEDED(pfd->GetResult(&psiResult))){
                PWSTR pszPath = nullptr;
                if (SUCCEEDED(psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))){
                    result = pszPath;
                    CoTaskMemFree(pszPath);
                }
                psiResult->Release();
            }
        }
        pfd->Release();
    }
    CoUninitialize();
    return result;
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
    //const wchar_t* subKey = L"Software\\YourCompany\\YourApp\\FolderDialog";
    const wchar_t* subKey = REGSTRY_KEYSTRING_FOLDER;
    std::wstring valueName = regValueName.empty() ? dlgTitle : regValueName;

    // 1) レジストリから前回のフォルダを読み出し
    std::wstring initialFolder;
    HKEY hKey = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, subKey, 0, nullptr,
        REG_OPTION_NON_VOLATILE, KEY_READ, nullptr,
        &hKey, nullptr) == ERROR_SUCCESS)
    {
        wchar_t buf[MAX_PATH] = { 0 };
        DWORD bufSize = sizeof(buf), type = REG_SZ;
        if (RegQueryValueExW(hKey,
            valueName.c_str(),
            nullptr,
            &type,
            reinterpret_cast<BYTE*>(buf),
            &bufSize) == ERROR_SUCCESS)
        {
            initialFolder = buf;
        }
        RegCloseKey(hKey);
    }

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
                    HKEY hKey2 = nullptr;
                    if (RegCreateKeyExW(HKEY_CURRENT_USER, subKey, 0, nullptr,
                        REG_OPTION_NON_VOLATILE, KEY_WRITE,
                        nullptr, &hKey2, nullptr) == ERROR_SUCCESS)
                    {
                        RegSetValueExW(hKey2,
                            valueName.c_str(),
                            0,
                            REG_SZ,
                            reinterpret_cast<const BYTE*>(result.c_str()),
                            static_cast<DWORD>((result.size() + 1) * sizeof(wchar_t)));
                        RegCloseKey(hKey2);
                    }
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
// LabelObjをファイル保存するための文字列生成関数
// 入力値はLabelObj
// 出力値は文字列 std::wstring
// UTF-8で保存する
// YOLO形式で保存する
// wchar_t → UTF-8 の変換（C++17 の場合）
std::string LabelsToString(
    const LabelObj& obj, 
	int mode = 0 // 0:default, 1:yolo
)
{
    std::ostringstream oss;
    // 数値とスペースだけなので、普通の narrow string で OK
    if (mode == 0)
    {
        oss << obj.ClassNum << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.X) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Y) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Width) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Height);
    }
	else if (mode == 1)
	{
        oss << obj.ClassNum << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.X + obj.rect.Width /2) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Y + obj.rect.Height/2) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Width) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Height);
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
	int mode = 0 // 0:default, 1:yolo
){
	// UTF-8で保存するための設定
	std::ofstream file(fileName, std::ios::binary);

	// ファイルオープン
	if (!file.is_open()) {
		return false; // ファイルオープン失敗
	}
	for (const auto& obj : objs) {
		file << LabelsToString(obj, mode) << std::endl;
	}
	file.close();
	return true;
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
std::wstring GetFileName(HWND hWnd, const std::wstring& title, int _rw)
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
    COMDLG_FILTERSPEC filter[] = {
        { L"YOLO",      L"*.txt" },
        { L"All Files", L"*.*"   }
    };
    pFileDialog->SetFileTypes(ARRAYSIZE(filter), filter);

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
std::wstring GetFileNameFromPath(const std::wstring& filePath)
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
//フォルダパスとファイル名と拡張子を指定して
//ファイルが存在すれば、そのファイルのフルパスを返す関数
//ファイルが存在しなければ、空文字""を返す
std::wstring GetFileNameFromPath(
    const std::wstring& folderpath, 
    const std::wstring& filename,
	const std::wstring& ext) // ".txt"など。"."を含むこと
{
	std::wstring _fn = GetFileNameFromPath(filename);

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
	std::wstring _fileName = GetFileNameFromPath(folderpath, imgObj.path, ext);
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
        while (file >> obj.ClassNum >> obj.rect.X >> obj.rect.Y >> obj.rect.Width >> obj.rect.Height)
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
            obj.rect.X = tmp_x - tmp_w / 2;
			obj.rect.Y = tmp_y - tmp_h / 2;
			obj.rect.Width = tmp_w;
            obj.rect.Height = tmp_h;
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
        std::wstring _fileName = GetFileNameFromPath(folderpath, imgObjs[i].path, ext);
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
            while (file >> obj.ClassNum >> obj.rect.X >> obj.rect.Y >> obj.rect.Width >> obj.rect.Height)
            {
				NormalizeRect(obj.rect); // 矩形の座標を正規化

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
				obj.rect.X = tmp_x - tmp_w / 2;
				obj.rect.Y = tmp_y - tmp_h / 2;
				obj.rect.Width = tmp_w;
				obj.rect.Height = tmp_h;

                NormalizeRect(obj.rect); // 矩形の座標を正規化
                
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
        std::wstring _fileName = GetFileNameFromPath(folderpath, imgObjs[i].path, ext);
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
                >> obj.rect.X >> obj.rect.Y
                >> obj.rect.Width >> obj.rect.Height)
            {
                NormalizeRect(obj.rect);

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
                obj.rect.X = tmp_x - tmp_w / 2;
                obj.rect.Y = tmp_y - tmp_h / 2;
                obj.rect.Width = tmp_w;
                obj.rect.Height = tmp_h;
                NormalizeRect(obj.rect);

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
    float x0 = obj.rect.X * GP.width;
    float y0 = obj.rect.Y * GP.height;
    float w = obj.rect.Width * GP.width;
    float h = obj.rect.Height * GP.height;

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
    float x0 = obj.rect.X * GP.width;
    float y0 = obj.rect.Y * GP.height;
    float w = obj.rect.Width * GP.width;
    float h = obj.rect.Height * GP.height;

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
// TaskDialogIndirectを使ってダイアログボックスを表示する関数
// Yes/Noボタンと、「次回から表示しない」を持つ
// 戻り値は、IDYES, IDNO, IDCHECKBOX
//int ShowDialogWithCheckbox(HWND hwnd, const std::wstring& _message, const std::wstring& _title)
//{
//	// ダイアログのボタンの設定
//	TASKDIALOG_BUTTON buttons[] = {
//		{ IDYES, L"Yes" },
//		{ IDNO, L"No" }
//	};
//	// ダイアログの設定
//	TASKDIALOGCONFIG tdc = { sizeof(tdc) };
//	tdc.hwndParent = hwnd;
//	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS;
//	tdc.pszWindowTitle = _title.c_str();
//	tdc.pszContent = _message.c_str();
//	tdc.cButtons = ARRAYSIZE(buttons);
//	tdc.pButtons = buttons;
//	tdc.nDefaultButton = IDYES;
//	// ダイアログを表示
//	int nButtonPressed = 0;
//	BOOL bCheckBoxChecked = FALSE;
//	TaskDialogIndirect(&tdc, &nButtonPressed, nullptr, &bCheckBoxChecked);
//	return nButtonPressed; // 戻り値は、IDYES, IDNO, IDCHECKBOX
//}
