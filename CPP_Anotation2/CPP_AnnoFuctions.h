#include "pch.h"

// 画像ファイルのパスの配列を取得する関数
int GetImgsPaths(const std::wstring& folderPath, std::vector<std::wstring>* imagePaths);
void NormalizeRect(RectF& r);
