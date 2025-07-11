#include "pch.h"

#include "framework.h"
#include "CPP_AnnoGblParams.h"

#pragma comment(lib, "comctl32.lib")


#define REGSTRY_KEYSTRING_FOLDER L"Software\\YourCompany\\YourApp\\FolderDialog"
#define REGSTRY_KEYSTRING_FILE L"Software\\YourCompany\\YourApp\\FileDialog"

// 画像ファイルのパスの配列を取得する関数
int GetImgsPaths(const std::wstring& folderPath, std::vector <ImgObject>& _imgObjs);

//画像ファイルをメモリにロード
int LoadImageFiles(const std::wstring& folderPath, std::vector <ImgObject>& _imgObjs);

//画像ファイルをメモリにロード 並列高速化
int LoadImageFilesMP(const std::wstring& folderPath, std::vector<ImgObject>& _imgObjs);

// 矩形の座標を正規化する関数
void NormalizeRect(Gdiplus::RectF& r);

// フォルダのパスを取得する関数
std::wstring GetFolderPath(HWND hWnd);
std::wstring GetFolderPathEx(HWND hWnd, const std::wstring& _currentFolder, const std::wstring& _title);
std::wstring GetFolderPathIF(HWND hWnd, const std::wstring& _currentFolder, const std::wstring& _title);
std::wstring GetFolderPathIFR(HWND hWnd, const std::wstring& dlgTitle, const std::wstring& regValueName = L"");

//std::wstring GetFileName(HWND hWnd);
std::wstring GetFileName(HWND hWnd, const std::wstring& title, int _rw);// _rw: 0=読み込み、1=書き込み

std::wstring ChangeFileExtension(const std::wstring& filePath, const std::wstring& newExt);
std::wstring GetOnlyFileNameFormPath(const std::wstring& filePath);
std::wstring GetFileNameFormPath(const std::wstring& filePath);		// ファイルパスからファイル名のみ抽出する関数 拡張子は残す
int MoveCurrentImageAndLabel(HWND hWnd, int imgIdx);


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
// LabelObjの文字列をファイル保存する関数
// 入力値はファイル名とconst std::vector<LabelObj>&
// 出力値は成功したらtrue、失敗したらfalse
bool SaveLabelsToFile(
	const std::wstring& fileName, 
	const std::vector<LabelObj>& objs,
	int _sc,
	float minimumsize, // 最小サイズ制限（デフォルトはなし）
	int mode //0:default, 1:yolo
);
///////////////////////////////////////////////////////////////////////
// LabelObjの文字列をファイル保存する関数
bool SaveLabelsToFileSingle(
	HWND hWnd, 
	size_t _idx,
	float minimumsize // 最小サイズ制限（デフォルトはなし）
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
//int GetIdxMouseOnRectEdge(
//	const POINT& pt,
//	std::vector<LabelObj>& objs,//対象の矩形の配列
//	EditMode& editMode,			//辺や頂点
//	int overlap
//);

size_t GetIdxMouseOnRectEdge(
	const POINT& pt,
	std::vector<LabelObj>& objs,
	EditMode& editMode,
	int overlap
);

////////////////////////////////////////////////////////////////////////////////
/// @brief クラス分類設定の読み書き
/// @param _filepath    入出力ファイルパス（ワイド文字列）
/// @param _clsNames    クラス名（wstring）
/// @param _clsColors   クラスカラー（Gdiplus::Color）
/// @param _dashStyles  線種（Gdiplus::DashStyle）
/// @param _clsPenWidths  ペン幅（int）
/// @param _rw          0=読み込み、1=書き込み
/// @return 0=成功、負値=エラー
///
int LoadClassification(
	const std::wstring& _filepath,
	std::vector<std::wstring>& _clsNames,
	std::vector<Gdiplus::Color>& _clsColors,
	std::vector<Gdiplus::DashStyle>& _dashStyles,
	std::vector<int>& _clsPenWidths,
	int _rw
);

///////////////////////////////////////////////////////////////////////
void SetStringToTitlleBar(HWND hWnd, std::wstring _imgfolder, std::wstring _labelfolder, int _Idx, int _Total);

//スケーリング
void SscalingRect(Gdiplus::RectF& r_in, Gdiplus::RectF& r_out, float scaleX, float scaleY);

///////////////////////////////////////////////////////////////////////
void DoPaint(HWND hWnd, WPARAM wParam, LPARAM lParam, size_t _idx);

///////////////////////////////////////////////////////////////////////
int  SaveAnnotations(HWND hWnd, std::wstring _title, float _sc);

///////////////////////////////////////////////////////////////////////
std::optional<size_t> jumpImgWithIgnoreBox(const std::vector<ImgObject>& imgObjs,size_t _startIdx = 0,float minW = 1.0f,float minH = 1.0f);
bool isIgnoreBox(const LabelObj& obj, float minW, float minH);

///////////////////////////////////////////////////////////////////////
std::wstring InsertSubFolder(const std::wstring& folderPath, const std::wstring& _sub);

