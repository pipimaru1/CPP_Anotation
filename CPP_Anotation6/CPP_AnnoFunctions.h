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
void NormalizeRect(RectF& r);

// �t�H���_�̃p�X���擾����֐�
std::wstring GetFolderPath(HWND hWnd);
std::wstring GetFolderPathEx(HWND hWnd, const std::wstring& _currentFolder, const std::wstring& _title);
std::wstring GetFolderPathIF(HWND hWnd, const std::wstring& _currentFolder, const std::wstring& _title);
std::wstring GetFolderPathIFR(HWND hWnd, const std::wstring& dlgTitle, const std::wstring& regValueName = L"");

//std::wstring GetFileName(HWND hWnd);
std::wstring GetFileName(HWND hWnd, const std::wstring& title, int _rw);// _rw: 0=�ǂݍ��݁A1=��������

std::wstring ChangeFileExtension(const std::wstring& filePath, const std::wstring& newExt);
std::wstring GetFileNameFromPath(const std::wstring& filePath);

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
// LabelObj���t�@�C���ۑ����邽�߂̕����񐶐��֐�
// ���͒l��LabelObj
// �o�͒l�͕����� std::wstring
// UTF-8�ŕۑ�����
// YOLO�`���ŕۑ�����
std::string LabelsToString(const LabelObj& obj);

///////////////////////////////////////////////////////////////////////
// LabelObj�̕�������t�@�C���ۑ�����֐�
// ���͒l�̓t�@�C������const std::vector<LabelObj>&
// �o�͒l�͐���������true�A���s������false
bool SaveLabelsToFile(
	const std::wstring& fileName, 
	const std::vector<LabelObj>& objs,
	int mode //0:default, 1:yolo
);


///////////////////////////////////////////////////////////////////////
//��`�̐���Ƀ}�E�X�J�[�\�������邩�ǂ����𔻒肷��֐�
EditMode IsMouseOnRectEdge(
	const POINT& pt,
	const LabelObj& obj,
	int overlap
);
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
//int ShowDialogWithCheckbox(HWND hwnd, const std::wstring& _message, const std::wstring& _title);
