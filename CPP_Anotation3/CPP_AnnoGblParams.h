#pragma once
///////////////////////////////////////////////////////
// �A�m�e�[�V�����N���X
struct AnnoObject
{
    //���W���
	Gdiplus::RectF  rect; // ��`�̍��W

    //�N���V�t�B�P�[�V�������
    std::wstring    ClassName; // ���x��
    int             CalassNum; // ���݂̃��x���C���f�b�N�X

	//�`����
    Gdiplus::Color  color; // �F
	int 		 penWidth; // �y���̕�
	Gdiplus::DashStyle dashStyle; // �_�b�V���X�^�C��

	// �R���X�g���N�^
    AnnoObject();
};



///////////////////////////////////////////////////////
// �O���[�o���ϐ��̒�`
// �ۑ�ł̓w�b�_�t�@�C���ɋL�ڂ����A���ނ̂��߂ɂ����ɋL�ڂ��܂��B
struct GlobalParams
{
    //�E�B���h�E�T�C�Y
    RECT rect_win;
    int width;
    int height;

    //�C���[�W�t�@�C���֘A
    std::vector<std::wstring>   imgPaths; // �摜�t�@�C���̃p�X���i�[����z��
    size_t                      imgIndex; // ���݂̉摜�C���f�b�N�X
	std::wstring                imgFolderPath; // ���݂̉摜�t�H���_

    // ��`�̔z��
	std::vector<AnnoObject>     objs; // ��`�̔z��
     
    // �}�E�X�h���b�O��
    AnnoObject                 obj_tmp; // ��`�̍��W
    bool    isDragging ; // �}�E�X�h���b�O�����ǂ���	
    size_t  rectIndex; // ���݂̋�`�C���f�b�N�X

    // �ΏۂƂ���摜�g���q�p�^�[��
    std::vector<std::wstring> IMAGE_EXTENSIONS;
    GlobalParams();

	// �N���V�t�B�P�[�V�����̖��O
    std::vector<std::wstring> ClsNames;
	int selectedClsIdx; // �I�����ꂽ�N���V�t�B�P�[�V�����̃C���f�b�N�X

};

extern GlobalParams GP;
