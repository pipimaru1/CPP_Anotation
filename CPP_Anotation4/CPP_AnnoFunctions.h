#include "pch.h"

#include "framework.h"
#include "CPP_AnnoGblParams.h"


// 画像ファイルのパスの配列を取得する関数
int GetImgsPaths(const std::wstring& folderPath, std::vector <ImgObject>& _imgObjs);
int LoadImageFiles(const std::wstring& folderPath, std::vector <ImgObject>& _imgObjs);


// 矩形の座標を正規化する関数
void NormalizeRect(RectF& r);

// フォルダのパスを取得する関数
std::wstring GetFolderPath(HWND hWnd);
std::wstring GetFileName(HWND hWnd);
std::wstring ChangeFileExtension(const std::wstring& filePath, const std::wstring& newExt);
std::wstring GetFileNameFromPath(const std::wstring& filePath);


///////////////////////////////////////////////////////////////////////
// Annotationをファイル保存するための文字列生成関数
// 入力値はAnnotation
// 出力値は文字列 std::wstring
// UTF-8で保存する
// YOLO形式で保存する
std::string AnnoObject2Str(const Annotation& obj);

///////////////////////////////////////////////////////////////////////
// Annotationの文字列をファイル保存する関数
// 入力値はファイル名とconst std::vector<Annotation>&
// 出力値は成功したらtrue、失敗したらfalse
bool SaveAnnoObjectsToFile(const std::wstring& fileName, const std::vector<Annotation>& objs);
