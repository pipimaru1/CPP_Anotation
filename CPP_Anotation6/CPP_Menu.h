#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <locale>
#include <sstream>
#include <filesystem>


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

#define DEF_MENU_FILE L"default.ini" // ���j���[�̃t�@�C���� 

struct MenuItemOnnx {
	std::wstring Id;          // �ԍ���
	std::wstring MenuStr;        // ���j���[��
	std::wstring OnnxPath;   // onnx�t�@�C���̃p�X
	std::wstring ClsNamePath;   // onnx�t�@�C���̃p�X

	std::wstring YoloType;    // yolo�̎��(v5,v8,v11��)
	int Resolution;          // �𑜓x(640, 1280, 1920��)
	int ClassCount;          // �N���X��
};

int loadmenu_onnx(const std::wstring& filename, std::vector<MenuItemOnnx>& menu);

int make_onnx_menus_by_id(
	HWND hWnd,
	UINT rootItemId,
	const std::vector<MenuItemOnnx>& menus,
	UINT IDM_START,
	const wchar_t* rootText /*�\����*/);
