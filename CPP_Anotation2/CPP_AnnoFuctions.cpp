#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFuctions.h"
#include "CPP_Anotation2.h"


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
int GetImgsPaths(const std::wstring& folderPath, std::vector<std::wstring>* imagePaths)
{
    imagePaths->clear();

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