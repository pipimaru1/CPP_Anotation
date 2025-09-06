#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <fstream>


//�t�@�C���̃��X�g���烁�j���[���쐬����
// �t�@�C���`��
// �ԍ����A���j���[���Aonnx�t�@�C���̃p�X�A�N���X���t�@�C���̃p�X, yolo�̎��(v5,v8,v11��)�A�𑜓x(640, 1280, 1920��)�A�N���X�� 
// ��
// ONNX[0], COCO��YOLOV5_S���f��,    D:/model/yolov5s.onnx,  D:/model/coco.txt, v5,  640,  80 
// ONNX[1], COCO��YOLOV8_S���f��,    D:/model/yolov8s.onnx,  D:/model/coco.txt, v8,  640,  80 
// ONNX[2], COCO��YOLOV11_S���f��,   D:/model/yolov11s.onnx, D:/model/coco.txt, v11, 640,  80 
// ONNX[2], COCO��YOLOV11_XP6���f��, D:/model/yolov11x6.onnx,D:/model/coco.txt, v11, 1280, 80 

// �ԍ����̓t�B�������₷�����邽�߂̂��̂ŁA���ۂ̏����ɂ͎g�p���Ȃ�
// �J���}�̌�̃X�y�[�X�͖���

#define DEF_MENU_FILE "default.ini" // ���j���[�̃t�@�C���� 

struct MenuItemOnnx {
	std::wstring Id;          // �ԍ���
	std::wstring MenuStr;        // ���j���[��
	std::wstring OnnxPath;   // onnx�t�@�C���̃p�X
	std::wstring ClsNamePath;   // onnx�t�@�C���̃p�X

	std::wstring YoloType;    // yolo�̎��(v5,v8,v11��)
	int Resolution;          // �𑜓x(640, 1280, 1920��)
	int ClassCount;          // �N���X��
};

/////////////////////////////////////////////////////////////////////
// ���j���[���t�@�C������ǂݍ���
int loadmenu_onnx(const char* filename, std::vector<MenuItemOnnx>& menu)
{
	std::wifstream _ifs(filename);
	if (_ifs)
	{
		std::wstring line;
		while (std::getline(_ifs, line))
		{
			if (line.empty()) 
				continue; // ��s�͖���
			if (line[0] == '#') 
				continue; // �R�����g�s�͖���
			//��s��
			MenuItemOnnx item;
			size_t pos = 0;
			size_t comma_pos = 0;
			int field_index = 0;
			while ((comma_pos = line.find(',', pos)) != std::string::npos)
			{
				std::wstring field = line.substr(pos, comma_pos - pos);
				field.erase(0, field.find_first_not_of(L" \t")); // �O�̋󔒂��폜
				field.erase(field.find_last_not_of(L" \t") + 1); // ��̋󔒂��폜
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
						break; // �s�v�ȃt�B�[���h�͖���
				}
				pos = comma_pos + 1;
				field_index++;
			}
			// �Ō�̃t�B�[���h������
			if (pos < line.size() && field_index == 5)
			{
				std::wstring field = line.substr(pos);
				field.erase(0, field.find_first_not_of(L" \t")); // �O�̋󔒂��폜
				field.erase(field.find_last_not_of(L" \t") + 1); // ��̋󔒂��폜
				item.ClassCount = std::stoi(field);
			}
			if (field_index >= 5) // �K�v�ȃt�B�[���h�������Ă���ꍇ�̂ݒǉ�
			{
				menu.push_back(item);
			}
		}
		return 0; // ����

	}

	else
	{
		return -1; // �t�@�C�����J���Ȃ�
	}
}

//////////////////////////////////////////////////////////////////////
// ���j���[���\�z����

int make_onnx_menus_by_id(
	HWND hWnd, 
	UINT rootItemId,
	const std::vector<MenuItemOnnx>& menus,
	UINT IDM_START,
	const wchar_t* rootText /*�\����*/)
{
	HMENU hMenu = GetMenu(hWnd);
	if (!hMenu) return -1;

	// �V�����T�u���j���[�����
	HMENU hSub = CreatePopupMenu();
	if (!hSub) return -1;

	// �q���ڂ�ǉ�
	for (size_t i = 0; i < menus.size(); ++i) {
		AppendMenuW(hSub, MF_STRING, IDM_START + static_cast<UINT>(i)+1, menus[i].MenuStr.c_str());
	}

	// ������ rootItemId ���u�|�b�v�A�b�v�v�ɒu�������ăT�u���j���[���Ԃ牺����
	// �� ��3������ MF_POPUP ��t���AuIDNewItem �� HMENU ��n���̂��|�C���g
	if (!ModifyMenuW(hMenu, rootItemId, MF_BYCOMMAND | MF_POPUP,
		(UINT_PTR)hSub, rootText ? rootText : L"Models")) {
		DestroyMenu(hSub);
		return -1;
	}

	DrawMenuBar(hWnd);
	return static_cast<int>(menus.size());
}
