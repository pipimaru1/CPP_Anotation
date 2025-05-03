#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_Anotation3.h"


///////////////////////////////////////////////////
//�R���X�g���N�^
GlobalParams::GlobalParams()
	:IMAGE_EXTENSIONS{ L"*.jpg", L"*.jpeg", L"*.png", L"*.bmp", L"*.gif" }
{
	/// �E�B���h�E�T�C�Y
	rect_win.left = 0;
	rect_win.top = 0;
	rect_win.right = 0;
	rect_win.bottom = 0;
	width = 0;
	height = 0;

	// �摜�t�@�C���֘A
	imgPaths.clear();
	imgIndex = 0;
	isDragging = false;

	// ��`�z��
	imgFolderPath.clear();
	rectIndex = 0;

	// �N���V�t�B�P�[�V�����̏�����
	ClsNames = { 
		L"person",
		L"forklift",
		L"tractor",
		L"driver",
		L"truck",
		L"excavator",
		L"wheelloder",
		L"grader",
		L"bulldozer",
		L"pallet",
		L"cargo",
		L"car"
	};
	
	// �I�����ꂽ�N���V�t�B�P�[�V�����̃C���f�b�N�X�̏�����
	// �Ȃ������l��-1�Ȃ̂��l���悤
	selectedClsIdx = -1; 
}

// �O���[�o���ϐ��̃C���X�^���X���쐬
GlobalParams GP;


///////////////////////////////////////////////////
AnnoObject::AnnoObject()
{
	// ��`�̏�����
	rect.X = 0;
	rect.Y = 0;
	rect.Width = 0;
	rect.Height = 0;
	// �N���V�t�B�P�[�V�����̏�����
	ClassName.clear();
	CalassNum = 0;


	// �`����̏�����
	color = Gdiplus::Color(255, 255, 255);
	penWidth = 2;
	dashStyle = Gdiplus::DashStyleSolid;


}