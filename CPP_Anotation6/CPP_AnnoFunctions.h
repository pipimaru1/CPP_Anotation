#include "pch.h"

#include "framework.h"
#include "CPP_AnnoGblParams.h"

#pragma comment(lib, "comctl32.lib")


#define REGSTRY_KEYSTRING_FOLDER L"Software\\YourCompany\\YourApp\\FolderDialog"
#define REGSTRY_KEYSTRING_FILE L"Software\\YourCompany\\YourApp\\FileDialog"

// �摜�t�@�C���̃p�X�̔z����擾����֐�
int GetImgsPaths(const std::wstring& folderPath, std::vector <ImgObject>& _imgObjs);

//�摜�t�@�C�����������Ƀ��[�h
int LoadImageFiles(const std::wstring& folderPath, std::vector <ImgObject>& _imgObjs);

//�摜�t�@�C�����������Ƀ��[�h ���񍂑���
int LoadImageFilesMP(const std::wstring& folderPath, std::vector<ImgObject>& _imgObjs);

// ��`�̍��W�𐳋K������֐�
void NormalizeRect(Gdiplus::RectF& r);

///////////////////////////////////////////////////////////////////////
// �t�H���_�[�p�X�����W�X�g������擾����֐�
//����������1��Ԃ�
int GetFolderPathfromReg(
	const std::wstring& _subKey,
	const std::wstring& _regValueName,
	std::wstring& _folderPath
);
///////////////////////////////////////////////////////////////////////
//�t�H���_�[�p�X�����W�X�g���ɕۑ�����֐�
// _subKey: ���W�X�g���̃T�u�L�[
// _regValueName: ���W�X�g���̒l��
// _folderPath: �ۑ�����t�H���_�[�p�X
void SaveFolderPathToReg(
	const std::wstring _subKey,
	const std::wstring _regValueName,
	const std::wstring _folderPath
);

// �t�H���_�̃p�X���擾����֐�
std::wstring GetFolderPath(HWND hWnd);
//std::wstring GetFolderPathEx(HWND hWnd, const std::wstring& _currentFolder, const std::wstring& _title);
std::wstring GetFolderPathEx(HWND hWnd, const std::wstring& _title);

//std::wstring GetFolderPathIF(HWND hWnd, const std::wstring& _currentFolder, const std::wstring& _title);
std::wstring GetFolderPathIFR(HWND hWnd, const std::wstring& dlgTitle, const std::wstring& regValueName = L"");

//std::wstring GetFileName(HWND hWnd);
//std::wstring GetFileName(HWND hWnd, const std::wstring& title, int _rw);// _rw: 0=�ǂݍ��݁A1=��������
std::wstring GetFileName(HWND hWnd, 
	const std::wstring& title, 
	COMDLG_FILTERSPEC filter[], 
	size_t filtersize, 
	int _rw);

std::wstring ChangeFileExtension(const std::wstring& filePath, const std::wstring& newExt);
std::wstring GetOnlyFileNameFormPath(const std::wstring& filePath);
std::wstring GetFileNameFormPath(const std::wstring& filePath);		// �t�@�C���p�X����t�@�C�����̂ݒ��o����֐� �g���q�͎c��
int MoveCurrentImageAndLabel(HWND hWnd, int imgIdx);


///////////////////////////////////////////////////////////////////////
// �֐� �A�m�e�[�V�����f�[�^�̓ǂݍ���
int LoadLabelFiles(
	std::vector<ImgObject>& imgObjs, //�f�[�^���i�[����ImgObject�N���X�̎Q��
	const std::wstring& folderpath, //�A�m�e�[�V�����t�@�C���̂���t�H���_�p�X
	const std::wstring& ext, //�A�m�e�[�V�����t�@�C���̊g���q
	int mode //0:default, 1:yolo
);
///////////////////////////////////////////////////////////////////////
// �֐� �A�m�e�[�V�����f�[�^�̓ǂݍ��� ���񍂑���
int LoadLabelFilesMP(
	std::vector<ImgObject>& imgObjs,
	const std::wstring& folderpath,
	const std::wstring& ext,
	int mode
);

///////////////////////////////////////////////////////////////////////
// LabelObj�̕�������t�@�C���ۑ�����֐�
// ���͒l�̓t�@�C������const std::vector<LabelObj>&
// �o�͒l�͐���������true�A���s������false
bool SaveLabelsToFile(
	const std::wstring& fileName, 
	const std::vector<LabelObj>& objs,
	int _sc,
	float minimumsize, // �ŏ��T�C�Y�����i�f�t�H���g�͂Ȃ��j
	int mode //0:default, 1:yolo
);
///////////////////////////////////////////////////////////////////////
// LabelObj�̕�������t�@�C���ۑ�����֐�
bool SaveLabelsToFileSingle(
	HWND hWnd, 
	size_t _idx,
	float minimumsize // �ŏ��T�C�Y�����i�f�t�H���g�͂Ȃ��j
	);

///////////////////////////////////////////////////////////////////////
//��`�̐���Ƀ}�E�X�J�[�\�������邩�ǂ����𔻒肷��֐�
//EditMode IsMouseOnRectEdge(
//	const POINT& pt,
//	const LabelObj& obj,
//	int overlap
//);
///////////////////////////////////////////////////////////////////////
//�}�E�X�J�[�\���Əd�Ȃ��`�̃C���f�b�N�X���擾����֐�
// �߂�l�̓}�E�X�J�[�\���Əd�Ȃ��`�̃C���f�b�N�X
// �d�Ȃ��`���Ȃ��ꍇ��-1��Ԃ�
// �}�E�X�J�[�\������`�̕ӏ�ɂ���ꍇ�́A�ӂ̃C���f�b�N�X��editMode�Ɋi�[����
//int GetIdxMouseOnRectEdge(
//	const POINT& pt,
//	std::vector<LabelObj>& objs,//�Ώۂ̋�`�̔z��
//	EditMode& editMode,			//�ӂⒸ�_
//	int overlap
//);

size_t GetIdxMouseOnRectEdge(
	const POINT& pt,
	std::vector<LabelObj>& objs,
	EditMode& editMode,
	int overlap
);

////////////////////////////////////////////////////////////////////////////////
/// @brief �N���X���ސݒ�̓ǂݏ���
/// @param _filepath    ���o�̓t�@�C���p�X�i���C�h������j
/// @param _clsNames    �N���X���iwstring�j
/// @param _clsColors   �N���X�J���[�iGdiplus::Color�j
/// @param _dashStyles  ����iGdiplus::DashStyle�j
/// @param _clsPenWidths  �y�����iint�j
/// @param _rw          0=�ǂݍ��݁A1=��������
/// @return 0=�����A���l=�G���[
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

//�X�P�[�����O
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

///////////////////////////////////////////////////////////////////////
void CheckMenues(HWND hWnd);

///////////////////////////////////////////////////////////////////////
int FixLabelBox_in_ImgObj(ImgObject& imgObj, float minW, float minH);

/////////////////////////////////////////////////////////////////////////
// �摜���N���C�A���g�Ƀt�B�b�g��������X�P�[���ƌ��_�i����j��Ԃ�
struct Viewport {
	float scale;           // �摜����ʂ̃X�P�[���iFit�~zoom�j
	Gdiplus::PointF origin;// �摜(0,0)����ʏ�̂ǂ��ɗ��邩
	Gdiplus::RectF dest;   // �`����`�iorigin �Ɖ摜�T�C�Y�~scale�j
};

Viewport ComputeViewport(const RECT& rcClient, UINT imgW, UINT imgH);
// ���K����`(�摜���W0..1) �� ��ʍ��W�i�r���[�|�[�g���j�ɕϊ�
Gdiplus::RectF NormRectToScreen(const Gdiplus::RectF& r01,
	const Viewport& vp, UINT imgW, UINT imgH);

// ��ʍ��W �� �摜���K�����W�i0..1�j�ɕϊ��i�q�b�g�e�X�g�p�j
Gdiplus::PointF ScreenPtToNorm(float sx, float sy,
	const Viewport& vp, UINT imgW, UINT imgH);

size_t GetIdxMouseOnRectEdgeVP(const POINT& pt,
	std::vector<LabelObj>& objs,
	EditMode& editMode,
	int overlap,
	const Gdiplus::RectF& view);

///////////////////////////////////////////////////////////////////////////////////////////
// �\���p���[�e�B���e�B�֐�
///////////////////////////////////////////////////////////////////////////////////////////
RectF FitImageToClientRect(int imgW, int imgH, const RECT& rcClient);
RectF NormToViewRect(const RectF& rNorm, const RectF& view);


void DrawLabelObjects(
	Graphics& g, 
	const std::vector<LabelObj>& objs,
	const RectF& view,
	Gdiplus::Font* font,
	bool _force_color = false, // �����I�ɐF��ς���
	Gdiplus::Color _color = Gdiplus::Color::White
);
void DrawLabelObject(
	Graphics& g, 
	const LabelObj& _obj, 
	const RectF& view,
	//int _penwidth,
	Gdiplus::Font* font,
	bool _force_color = false, // �����I�ɐF��ς���
	Gdiplus::Color _color = Gdiplus::Color::White
);


