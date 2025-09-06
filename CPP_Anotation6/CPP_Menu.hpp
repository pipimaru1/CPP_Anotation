#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <fstream>


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

#define DEF_MENU_FILE "default.ini" // メニューのファイル名 

struct MenuItemOnnx {
	std::wstring Id;          // 番号名
	std::wstring MenuStr;        // メニュー名
	std::wstring OnnxPath;   // onnxファイルのパス
	std::wstring ClsNamePath;   // onnxファイルのパス

	std::wstring YoloType;    // yoloの種類(v5,v8,v11等)
	int Resolution;          // 解像度(640, 1280, 1920等)
	int ClassCount;          // クラス数
};

/////////////////////////////////////////////////////////////////////
// メニューをファイルから読み込む
int loadmenu_onnx(const char* filename, std::vector<MenuItemOnnx>& menu)
{
	std::wifstream _ifs(filename);
	if (_ifs)
	{
		std::wstring line;
		while (std::getline(_ifs, line))
		{
			if (line.empty()) 
				continue; // 空行は無視
			if (line[0] == '#') 
				continue; // コメント行は無視
			//一行分
			MenuItemOnnx item;
			size_t pos = 0;
			size_t comma_pos = 0;
			int field_index = 0;
			while ((comma_pos = line.find(',', pos)) != std::string::npos)
			{
				std::wstring field = line.substr(pos, comma_pos - pos);
				field.erase(0, field.find_first_not_of(L" \t")); // 前の空白を削除
				field.erase(field.find_last_not_of(L" \t") + 1); // 後の空白を削除
				switch (field_index)
				{
					case 0: 
						item.Id = field; 
						break;
					case 1: 
						item.MenuStr = field;
						break;
					case 2: 
						item.OnnxPath = field; 
						break;
					case 3:
						item.ClsNamePath = field;
						break;
					case 4:
						item.YoloType = field; 
						break;
					case 5: 
						item.Resolution = std::stoi(field); 
						break;
					case 6: 
						item.ClassCount = std::stoi(field); 
						break;
					default: 
						break; // 不要なフィールドは無視
				}
				pos = comma_pos + 1;
				field_index++;
			}
			// 最後のフィールドを処理
			if (pos < line.size() && field_index == 5)
			{
				std::wstring field = line.substr(pos);
				field.erase(0, field.find_first_not_of(L" \t")); // 前の空白を削除
				field.erase(field.find_last_not_of(L" \t") + 1); // 後の空白を削除
				item.ClassCount = std::stoi(field);
			}
			if (field_index >= 5) // 必要なフィールドが揃っている場合のみ追加
			{
				menu.push_back(item);
			}
		}
		return 0; // 成功

	}

	else
	{
		return -1; // ファイルが開けない
	}
}

//////////////////////////////////////////////////////////////////////
// メニューを構築する

int make_onnx_menus_by_id(
	HWND hWnd, 
	UINT rootItemId,
	const std::vector<MenuItemOnnx>& menus,
	UINT IDM_START,
	const wchar_t* rootText /*表示名*/)
{
	HMENU hMenu = GetMenu(hWnd);
	if (!hMenu) return -1;

	// 新しいサブメニューを作る
	HMENU hSub = CreatePopupMenu();
	if (!hSub) return -1;

	// 子項目を追加
	for (size_t i = 0; i < menus.size(); ++i) {
		AppendMenuW(hSub, MF_STRING, IDM_START + static_cast<UINT>(i)+1, menus[i].MenuStr.c_str());
	}

	// 既存の rootItemId を「ポップアップ」に置き換えてサブメニューをぶら下げる
	// ※ 第3引数に MF_POPUP を付け、uIDNewItem に HMENU を渡すのがポイント
	if (!ModifyMenuW(hMenu, rootItemId, MF_BYCOMMAND | MF_POPUP,
		(UINT_PTR)hSub, rootText ? rootText : L"Models")) {
		DestroyMenu(hSub);
		return -1;
	}

	DrawMenuBar(hWnd);
	return static_cast<int>(menus.size());
}
