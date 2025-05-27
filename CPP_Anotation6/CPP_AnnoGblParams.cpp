#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_Anotation6.h"

///////////////////////////////////////////////////
//�R���X�g���N�^
GlobalParams::GlobalParams()
	:IMAGE_EXTENSIONS{ L"*.jpg", L"*.jpeg", L"*.png", L"*.bmp", L"*.gif" },
	fontFamily(nullptr), font(nullptr),
	imgIdx(0)
{
	// �摜�t�@�C���֘A
	imgObjs.clear(); // �摜�t�@�C���̃p�X�Ƌ�`�̔z��
	imgIdx = 0;

	/// �E�B���h�E�T�C�Y
	rect_win.left = 0;
	rect_win.top = 0;
	rect_win.right = 0;
	rect_win.bottom = 0;
	width = 0;
	height = 0;


	//�}�E�X
	//makeBox = false;
	dgMode = DragMode::None; // �h���b�O���[�h
	isMouseMoving = false;
	g_prevPt = { -1, -1 };
    Overlap = 5; // �}�E�X�I�[�o�[�̗T�x

	// ��`�z��
	imgFolderPath.clear();
	//objIdx = 0;

    labelFolderPath = L""; // ���݂̃��x���t�H���_
    imgFolderPath = L""; // ���݂̉摜�t�H���_

	// �N���V�t�B�P�[�V�����̏����� ������ύX���邱�ƂŁA�N���X����F��ύX�ł���
	ClsNames = { 
		L"person",
		L"truck",
		L"car",
		L"cat",
		L"dog"
	};
	ClsColors = {
		Gdiplus::Color(255, 255, 0),
		Gdiplus::Color(255, 0, 0),
		Gdiplus::Color(0, 255, 0),
		Gdiplus::Color(0, 0, 255),
		Gdiplus::Color(255, 0, 255)
	};
	ClsDashStyles = {
		Gdiplus::DashStyleSolid,
		Gdiplus::DashStyleDash,
		Gdiplus::DashStyleDot,
		Gdiplus::DashStyleDashDot,
		Gdiplus::DashStyleDashDotDot
	};
	ClsPenWidths ={
		2, 3, 4, 5, 6
	};

	// �I�����ꂽ�N���V�t�B�P�[�V�����̃C���f�b�N�X�̏�����
	// �Ȃ������l��-1�Ȃ̂��l���悤
	selectedClsIdx = -1; 
}

///////////////////////////////////////////////////
void GlobalParams::InitFont()
{
	fontFamily = new Gdiplus::FontFamily(L"Arial");
	font = new Gdiplus::Font(fontFamily, 12.0f,
		Gdiplus::FontStyleRegular,
		Gdiplus::UnitPixel);
}

void GlobalParams::DestroyFont()
{
	delete font;
	delete fontFamily;
	font = nullptr;
	fontFamily = nullptr;
}

GlobalParams::~GlobalParams()
{
	DestroyFont();
}



///////////////////////////////////////////////////
LabelObj::LabelObj()
{
	// ��`�̏�����
	Rct.X = 0;
	Rct.Y = 0;
	Rct.Width = 0;
	Rct.Height = 0;
	// �N���V�t�B�P�[�V�����̏�����
	ClassName.clear();
	ClassNum = 0;
	mOver = false;

	// �`����̏�����
	color = Gdiplus::Color(255, 255, 255);
	penWidth = 2;
	dashStyle = Gdiplus::DashStyleSolid;
}

///////////////////////////////////////////////////

ImgObject::ImgObject() = default;
ImgObject::~ImgObject() = default;


