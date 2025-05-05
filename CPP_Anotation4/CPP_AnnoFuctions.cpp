#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFuctions.h"
#include "CPP_Anotation4.h"


///////////////////////////////////////////////////////////////////////
// �摜�t�@�C�����ǂ����𔻒肷��֐�
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
// �t�H���_�̉摜�t�@�C�����擾����֐�
int GetImgsPaths(const std::wstring& folderPath, std::vector <ImgObject>& _imgObjs)
{
    _imgObjs.clear();

    std::wstring searchPath = folderPath;
    if (!searchPath.empty() && searchPath.back() != L'\\')
        searchPath += L'\\';
    searchPath += L"*.*";  // �S�t�@�C���Ώ�

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0; // �t�H���_��������Ȃ�
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
// ��`�̍��W�𐳋K������֐�
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
// �t�H���_�I���_�C�A���O��\������֐�
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
// Annotation���t�@�C���ۑ����邽�߂̕����񐶐��֐�
// ���͒l��Annotation
// �o�͒l�͕����� std::wstring
// UTF-8�ŕۑ�����
// YOLO�`���ŕۑ�����
// wchar_t �� UTF-8 �̕ϊ��iC++17 �̏ꍇ�j
std::string AnnoObject2Str(const Annotation& obj)
{
    std::ostringstream oss;
    // ���l�ƃX�y�[�X�����Ȃ̂ŁA���ʂ� narrow string �� OK
    oss << obj.CalassNum << ' '
        << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.X) << ' '
        << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Y) << ' '
        << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Width) << ' '
        << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Height);
    return oss.str();  // �Ԃ�l�� std::string �� UTF-8 �G���R�[�h�ς�
}

///////////////////////////////////////////////////////////////////////
// Annotation�̕�������t�@�C���ۑ�����֐�
// ���͒l�̓t�@�C������const std::vector<Annotation>&
// �o�͒l�͐���������true�A���s������false
bool SaveAnnoObjectsToFile(const std::wstring& fileName, const std::vector<Annotation>& objs)
{
	// UTF-8�ŕۑ����邽�߂̐ݒ�
	std::ofstream file(fileName, std::ios::binary);

	// �t�@�C���I�[�v��
	if (!file.is_open()) {
		return false; // �t�@�C���I�[�v�����s
	}
	for (const auto& obj : objs) {
		file << AnnoObject2Str(obj) << std::endl;
	}
	file.close();
	return true;
}

///////////////////////////////////////////////////////////////////////
// �t�@�C���ۑ����邽�߂̃t�@�C�������_�C�A���O�{�b�N�X�Ŏ擾����֐�
// ���͒l�̓E�B���h�E�n���h��
// �o�͒l�̓t�@�C���� std::wstring
std::wstring GetFileName(HWND hWnd)
{
	std::wstring fileName;
	IFileDialog* pFileDialog = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pFileDialog));
	if (SUCCEEDED(hr)) {
		// �t�B���^��ݒ�
		COMDLG_FILTERSPEC filter[] = {
			{ L"YOLO", L"*.txt" },
			{ L"All Files", L"*.*" }
		};
		pFileDialog->SetFileTypes(ARRAYSIZE(filter), filter);
		// �_�C�A���O��\��
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
// �t�@�C���p�X����g���q���w��̊g���q�ɕύX����֐�
// ���͒l�̓t�@�C���p�X�Ɗg���q
// �o�͒l�͕ύX��̃t�@�C���p�X std::wstring
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
// �t�@�C���p�X����g���q����菜���֐�
// ���͒l�̓t�@�C���p�X
// �o�͒l�͊g���q����菜�����t�@�C���p�X std::wstring
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
// �t�@�C���p�X����t�@�C�����̂ݒ��o����֐�
// �g���q���폜
// ���͒l�̓t�@�C���p�X
// �o�͒l�̓t�@�C���� std::wstring
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
