#pragma once

enum class DragMode { None, MakeBox, ReBox, dummy};
enum class EditMode { None, Left, Right, Top, Bottom, LeftTop, RightTop, LeftBottom, RightBottom };

///////////////////////////////////////////////////////
// �A�m�e�[�V�����N���X
class LabelObj
{
public:
    //���W���
    Gdiplus::RectF  Rct; //���K�����ꂽ��`
	Gdiplus::RectF  Rct_Scale; // �X�P�[�����ꂽ��` 

public:
    //�N���V�t�B�P�[�V�������
    std::wstring    ClassName; // ���x��
    int             ClassNum; // ���݂̃��x���C���f�b�N�X

	//�`����
    Gdiplus::Color      color; // �F
	int 		        penWidth; // �y���̕�
	Gdiplus::DashStyle  dashStyle; // �_�b�V���X�^�C��

    bool mOver; //�}�E�X���I�[�o�[���b�v���Ă���Ƃ��̕ӂ�\��

	// �R���X�g���N�^
    LabelObj();
};

///////////////////////////////////////////////////////
// �摜�N���X
// 1�̉摜�̃p�X�Ƌ�`�̔z������N���X
struct ImgObject
{
    std::wstring            path;     // �摜�̃p�X
    std::vector<LabelObj>   objs;     // ��`�̔z��
    size_t                  objIdx{0};// ���݂̋�`�C���f�b�N�X
    std::unique_ptr<Gdiplus::Image> image;  // ���|�C���^�͋֎~�I
    int mOverIdx = -1;// �}�E�X�I�[�o�[�̃C���f�b�N�X
	bool isEdited = false; // �ҏW���ꂽ���ǂ���

    // �R���X�g���N�^�̓f�t�H���g��OK
    ImgObject();
    // �f�X�g���N�^���f�t�H���g�łn�j
    ~ImgObject();

    // �R�s�[�͋֎~�i���|�C���^��d�����h���j
    ImgObject(const ImgObject&) = delete;
    ImgObject& operator=(const ImgObject&) = delete;

    // ���[�u�̓f�t�H���g�łn�j�iunique_ptr �����[�u�����j
    ImgObject(ImgObject&&) noexcept = default;
    ImgObject& operator=(ImgObject&&) noexcept = default;

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
    std::wstring                labelFolderPath; // ���݂̃��x���t�H���_

    // �}�E�X�h���b�O��
    LabelObj  tmpLabel;    // ��`�̍��W
	DragMode dgMode = DragMode::None; // �h���b�O���[�h

	//�}�E�X�ړ���
	bool        isMouseMoving; // �}�E�X�ړ������ǂ���
    POINT       g_prevPt;
    int         Overlap; // �}�E�X�I�[�o�[�̗T�x

    //��`�h���b�O
	int activeIdx   = -1; // �h���b�O���̋�`�̃C���f�b�N�X
	EditMode edMode = EditMode::None;// �}�E�X�I�[�o�[���̋�`�̕ӂ�\��
    Gdiplus::PointF prevMouse;   // �O��̃}�E�X���W

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

	int selectedClsIdx; // �I�����ꂽ�N���V�t�B�P�[�V�����̃C���f�b�N�X

    // �t�H���g������x�������郁�\�b�h
    void InitFont();
    void DestroyFont();

    //�R���X�g���N�^
    GlobalParams();
    ~GlobalParams();

};

extern GlobalParams GP;
