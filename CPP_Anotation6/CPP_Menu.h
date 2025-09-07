#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <locale>
#include <sstream>
#include <filesystem>


//ファイルのリストからメニューを作成する
// ファイル形式
// 番号名、メニュー名、onnxファイルのパス、クラス名ファイルのパス, yoloの種類(v5,v8,v11等)、解像度(640, 1280, 1920等)、クラス数 
// 例
// ONNX[0], COCOのYOLOV5_Sモデル,    D:/model/yolov5s.onnx,  D:/model/coco.txt, v5,  640,  80 
// ONNX[1], COCOのYOLOV8_Sモデル,    D:/model/yolov8s.onnx,  D:/model/coco.txt, v8,  640,  80 
// ONNX[2], COCOのYOLOV11_Sモデル,   D:/model/yolov11s.onnx, D:/model/coco.txt, v11, 640,  80 
// ONNX[2], COCOのYOLOV11_XP6モデル, D:/model/yolov11x6.onnx,D:/model/coco.txt, v11, 1280, 80 

// 番号名はフィルを見やすくするためのもので、実際の処理には使用しない
// カンマの後のスペースは無視

#define DEF_MENU_FILE L"default.ini" // メニューのファイル名 

struct MenuItemOnnx {
	std::wstring Id;          // 番号名
	std::wstring MenuStr;        // メニュー名
	std::wstring OnnxPath;   // onnxファイルのパス
	std::wstring ClsNamePath;   // onnxファイルのパス

	std::wstring YoloType;    // yoloの種類(v5,v8,v11等)
	int Resolution;          // 解像度(640, 1280, 1920等)
	int ClassCount;          // クラス数
};

int loadmenu_onnx(const std::wstring& filename, std::vector<MenuItemOnnx>& menu);

int make_onnx_menus_by_id(
	HWND hWnd,
	UINT rootItemId,
	const std::vector<MenuItemOnnx>& menus,
	UINT IDM_START,
	const wchar_t* rootText /*表示名*/);
