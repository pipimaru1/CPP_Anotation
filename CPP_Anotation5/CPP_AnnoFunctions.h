#include "pch.h"

#include "framework.h"
#include "CPP_AnnoGblParams.h"


// 画像ファイルのパスの配列を取得する関数
int GetImgsPaths(const std::wstring& folderPath, std::vector <ImgObject>& _imgObjs);

//画像ファイルをメモリにロード
int LoadImageFiles(const std::wstring& folderPath, std::vector <ImgObject>& _imgObjs);

//画像ファイルをメモリにロード 並列高速化
int LoadImageFilesMP(const std::wstring& folderPath, std::vector<ImgObject>& _imgObjs);

// 矩形の座標を正規化する関数
void NormalizeRect(RectF& r);

// フォルダのパスを取得する関数
std::wstring GetFolderPath(HWND hWnd);
std::wstring GetFolderPathEx(HWND hWnd, const std::wstring& _currentFolder, const std::wstring& _title);
std::wstring GetFolderPathIF(HWND hWnd, const std::wstring& _currentFolder, const std::wstring& _title);
std::wstring GetFolderPathIFR(HWND hWnd, const std::wstring& dlgTitle, const std::wstring& regValueName = L"");

std::wstring GetFileName(HWND hWnd);
std::wstring ChangeFileExtension(const std::wstring& filePath, const std::wstring& newExt);
std::wstring GetFileNameFromPath(const std::wstring& filePath);

///////////////////////////////////////////////////////////////////////
// 関数 アノテーションデータの読み込み
int LoadLabelFiles(
	std::vector<ImgObject>& imgObjs, //データを格納するImgObjectクラスの参照
	const std::wstring& folderpath, //アノテーションファイルのあるフォルダパス
	const std::wstring& ext, //アノテーションファイルの拡張子
	int mode //0:default, 1:yolo
);
///////////////////////////////////////////////////////////////////////
// 関数 アノテーションデータの読み込み 並列高速化
int LoadLabelFilesMP(
	std::vector<ImgObject>& imgObjs,
	const std::wstring& folderpath,
	const std::wstring& ext,
	int mode
);

///////////////////////////////////////////////////////////////////////
// LabelObjをファイル保存するための文字列生成関数
// 入力値はLabelObj
// 出力値は文字列 std::wstring
// UTF-8で保存する
// YOLO形式で保存する
std::string LabelsToString(const LabelObj& obj);

///////////////////////////////////////////////////////////////////////
// LabelObjの文字列をファイル保存する関数
// 入力値はファイル名とconst std::vector<LabelObj>&
// 出力値は成功したらtrue、失敗したらfalse
bool SaveLabelsToFile(
	const std::wstring& fileName, 
	const std::vector<LabelObj>& objs,
	int mode //0:default, 1:yolo
);


///////////////////////////////////////////////////////////////////////
//矩形の線上にマウスカーソルがあるかどうかを判定する関数
EditMode IsMouseOnRectEdge(
	const POINT& pt,
	const LabelObj& obj,
	int overlap
);
///////////////////////////////////////////////////////////////////////
//マウスカーソルと重なる矩形のインデックスを取得する関数
// 戻り値はマウスカーソルと重なる矩形のインデックス
// 重なる矩形がない場合は-1を返す
// マウスカーソルが矩形の辺上にある場合は、辺のインデックスをeditModeに格納する
int GetIdxMouseOnRectEdge(
	const POINT& pt,
	std::vector<LabelObj>& objs,//対象の矩形の配列
	EditMode& editMode,			//辺や頂点
	int overlap
);
