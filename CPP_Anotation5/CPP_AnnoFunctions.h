#include "pch.h"

#include "framework.h"
#include "CPP_AnnoGblParams.h"


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
std::wstring GetFileName(HWND hWnd);
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
// Annotation���t�@�C���ۑ����邽�߂̕����񐶐��֐�
// ���͒l��Annotation
// �o�͒l�͕����� std::wstring
// UTF-8�ŕۑ�����
// YOLO�`���ŕۑ�����
std::string AnnoObject2Str(const Annotation& obj);

///////////////////////////////////////////////////////////////////////
// Annotation�̕�������t�@�C���ۑ�����֐�
// ���͒l�̓t�@�C������const std::vector<Annotation>&
// �o�͒l�͐���������true�A���s������false
bool SaveAnnoObjectsToFile(
	const std::wstring& fileName, 
	const std::vector<Annotation>& objs,
	int mode //0:default, 1:yolo
);
