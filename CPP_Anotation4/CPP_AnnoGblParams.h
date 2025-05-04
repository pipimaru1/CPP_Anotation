#pragma once
///////////////////////////////////////////////////////
// �A�m�e�[�V�����N���X
struct Annotation
{
    //���W���
	Gdiplus::RectF  rect; // ��`�̍��W

    //�N���V�t�B�P�[�V�������
    std::wstring    ClassName; // ���x��
    int             CalassNum; // ���݂̃��x���C���f�b�N�X

	//�`����
    Gdiplus::Color      color; // �F
	int 		        penWidth; // �y���̕�
	Gdiplus::DashStyle  dashStyle; // �_�b�V���X�^�C��

	// �R���X�g���N�^
    Annotation();
};

///////////////////////////////////////////////////////
// �摜�N���X
// 1�̉摜�̃p�X�Ƌ�`�̔z������N���X
struct ImgObject
{
    std::wstring path;         // �摜�̃p�X
    std::vector<Annotation> objs; // ��`�̔z��
    size_t      objIdx;           // ���݂̋�`�C���f�b�N�X

    // �R���X�g���N�^
    ImgObject();
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
	std::vector<ImgObject>      imgObjs; // �摜�t�@�C���̃p�X�Ƌ�`�̔z��
    size_t                      imgIdx; // ���݂̉摜�C���f�b�N�X
	std::wstring                imgFolderPath; // ���݂̉摜�t�H���_

    // ��`�̔z��
	//std::vector<Annotation>     objs; // ��`�̔z��
    //size_t      objIdx;     // ���݂̋�`�C���f�b�N�X

    // �}�E�X�h���b�O��
    Annotation  anno_tmp;    // ��`�̍��W
    bool        isDragging; // �}�E�X�h���b�O�����ǂ���	

    // �ΏۂƂ���摜�g���q�p�^�[��
    std::vector<std::wstring>   IMAGE_EXTENSIONS;

	// �N���V�t�B�P�[�V�����̖��O
    std::vector<std::wstring>       ClsNames;
    std::vector<Gdiplus::Color>     ClsColors;
    std::vector<Gdiplus::DashStyle> ClsDashStyles;
	std::vector<int>                ClsPenWidths;

    // �t�H���g�֘A�̓|�C���^��
    Gdiplus::FontFamily* fontFamily;
    Gdiplus::Font* font;
    //Gdiplus::SolidBrush  fontBrush;

	int selectedClsIdx; // �I�����ꂽ�N���V�t�B�P�[�V�����̃C���f�b�N�X

    // �t�H���g������x�������郁�\�b�h
    void InitFont();
    void DestroyFont();

    //�R���X�g���N�^
    GlobalParams();
    ~GlobalParams();

};

extern GlobalParams GP;
