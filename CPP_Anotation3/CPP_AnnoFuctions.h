#include "pch.h"

// �摜�t�@�C���̃p�X�̔z����擾����֐�
int GetImgsPaths(const std::wstring& folderPath, std::vector<std::wstring>* imagePaths);

// ��`�̍��W�𐳋K������֐�
void NormalizeRect(RectF& r);

// �t�H���_�̃p�X���擾����֐�
std::wstring GetFolderPath(HWND hWnd);