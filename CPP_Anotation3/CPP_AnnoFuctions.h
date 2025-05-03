#include "pch.h"

// 画像ファイルのパスの配列を取得する関数
int GetImgsPaths(const std::wstring& folderPath, std::vector<std::wstring>* imagePaths);

// 矩形の座標を正規化する関数
void NormalizeRect(RectF& r);

// フォルダのパスを取得する関数
std::wstring GetFolderPath(HWND hWnd);