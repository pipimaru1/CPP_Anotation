#pragma once


///////////////////////////////////////////////////////
// �O���[�o���ϐ��̒�`
// �ۑ�ł̓w�b�_�t�@�C���ɋL�ڂ����A���ނ̂��߂ɂ����ɋL�ڂ��܂��B
struct GlobalParams
{
    //�E�B���h�E�T�C�Y
    RECT rect_win;
    int width;
    int height;

    //�C���[�W�t�@�C��
    std::vector<std::wstring> imgPaths; // �摜�t�@�C���̃p�X���i�[����z��
    size_t imgIndex; // ���݂̉摜�C���f�b�N�X

    // ��`�̔z��
    std::vector<Gdiplus::RectF> rects; // ��`�̔z��
    // �}�E�X�h���b�O��
    Gdiplus::RectF rect_tmp; // ��`�̍��W
    bool isDragging = false; // �}�E�X�h���b�O�����ǂ���	
    size_t rectIndex = 0; // ���݂̋�`�C���f�b�N�X

    // �ΏۂƂ���摜�g���q�p�^�[��
    std::vector<std::wstring> IMAGE_EXTENSIONS;
    GlobalParams();
};

extern GlobalParams GP;
