#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFuctions.h"
#include "CPP_Anotation4.h"


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
				ImgObject _imgobj;
                _imgobj.path = folderPath;

                if (!_imgobj.path.empty() && _imgobj.path.back() != L'\\')
                    _imgobj.path += L'\\';
                _imgobj.path += fileName;
                _imgObjs.push_back(_imgobj);
            }
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    return static_cast<int>(_imgObjs.size());
}

///////////////////////////////////////////////////////////////////////
// 矩形の座標を正規化する関数
void NormalizeRect(RectF& r) {
    if (r.Width < 0) {
        r.X += r.Width;
        r.Width = -r.Width;
    }
    if (r.Height < 0) {
        r.Y += r.Height;
        r.Height = -r.Height;
    }
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
// Annotationをファイル保存するための文字列生成関数
// 入力値はAnnotation
// 出力値は文字列 std::wstring
// UTF-8で保存する
// YOLO形式で保存する
// wchar_t → UTF-8 の変換（C++17 の場合）
std::string AnnoObject2Str(const Annotation& obj)
{
    std::ostringstream oss;
    // 数値とスペースだけなので、普通の narrow string で OK
    oss << obj.CalassNum << ' '
        << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.X) << ' '
        << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Y) << ' '
        << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Width) << ' '
        << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Height);
    return oss.str();  // 返り値の std::string は UTF-8 エンコード済み
}

///////////////////////////////////////////////////////////////////////
// Annotationの文字列をファイル保存する関数
// 入力値はファイル名とconst std::vector<Annotation>&
// 出力値は成功したらtrue、失敗したらfalse
bool SaveAnnoObjectsToFile(const std::wstring& fileName, const std::vector<Annotation>& objs)
{
	// UTF-8で保存するための設定
	std::ofstream file(fileName, std::ios::binary);

	// ファイルオープン
	if (!file.is_open()) {
		return false; // ファイルオープン失敗
	}
	for (const auto& obj : objs) {
		file << AnnoObject2Str(obj) << std::endl;
	}
	file.close();
	return true;
}

///////////////////////////////////////////////////////////////////////
// ファイル保存するためのファイル名をダイアログボックスで取得する関数
// 入力値はウィンドウハンドル
// 出力値はファイル名 std::wstring
std::wstring GetFileName(HWND hWnd)
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
