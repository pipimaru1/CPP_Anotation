#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric> // Add this include at the top of the file

#include "CPP_AnnoGblParams.h" // LabelObj, GlobalParams(GP)

/*
enum DnnModelKind {
    DNN_KIND_AUTO = 0,   // �o�͌`�󂩂玩������iYOLO���o or �摜���ށj
    DNN_KIND_YOLO,       // YOLO�n���o�iv5/v7/v8 �Ȃǁj
    DNN_KIND_CLS         // �摜���ށi1xC or 1xCx1x1 ���W�b�g�j
};

enum BoxFormat {
    BOX_XYWH_NORM = 0,   // ���K��xywh�i����N�_�E[0..1]�j
    BOX_XYWH_PIXEL,      // �s�N�Z��xywh�i����N�_�j
    BOX_XYXY_PIXEL       // �s�N�Z��xyxy�iminmax�j
};

struct DnnOptions {
    int backend = cv::dnn::DNN_BACKEND_OPENCV; // CUDA��: DNN_BACKEND_CUDA
    int target = cv::dnn::DNN_TARGET_CPU;     // CUDA��: DNN_TARGET_CUDA
    bool reuse_net = true;                     // ONNX���L���b�V�����Ďg����
};

struct YoloConfig {
    int   inputW = 640;
    int   inputH = 640;
    bool  swapRB = true;
    float scale = 1.0f / 255.0f;
    cv::Scalar mean = cv::Scalar(0, 0, 0);
    bool  letterbox = true;
    float confThreshold = 0.25f;
    float nmsThreshold = 0.45f;
    BoxFormat boxFormat = BOX_XYWH_NORM;
};

struct ClsConfig {
    int   inputW = 224;
    int   inputH = 224;
    bool  swapRB = true;
    float scale = 1.0f / 255.0f;
    cv::Scalar mean = cv::Scalar(0, 0, 0);
    int   topk = 5; // ���k��
};

struct DetBox {
    int   classId;
    float conf;
    cv::Rect2f box; // boxFormat�ɏ]���i�f�t�H���g: ���K��xywh�j
};

struct ClsScore {
    int   classId;
    float prob;
};

struct DnnParams {
    DnnModelKind kind = DNN_KIND_AUTO;
    DnnOptions   opt;
    YoloConfig   yolo;
    ClsConfig    cls;
};

struct DnnResult {
    std::vector<DetBox> dets; // ���o������Ίi�[
    std::vector<ClsScore> cls;// ���ނ�����Ίi�[�itop-k�j
    double preprocessMs = 0.0;
    double inferMs = 0.0;
    double postMs = 0.0;
};

// ����1�{��OK�F�摜Mat�AONNX�p�X�A�e��p�����[�^ �� ���o/���ނ̔z���Ԃ�
DnnResult DnnInfer(const cv::Mat& bgr,
    const std::wstring& onnxPath,
    const DnnParams& params = DnnParams());
*/

// ���_�o�b�N�G���h/�^�[�Q�b�g
struct DnnOptions {
    int backend = cv::dnn::DNN_BACKEND_OPENCV; // CUDA��: DNN_BACKEND_CUDA
    int target = cv::dnn::DNN_TARGET_CPU;     // CUDA��: DNN_TARGET_CUDA
    bool reuse_net = true;                     // ONNX���L���b�V��
};

// YOLO�O�㏈���p�����[�^
struct YoloConfig {
    int   inputW = 640;
    int   inputH = 640;
    bool  swapRB = true;
    float scale = 1.0f / 255.0f;
    cv::Scalar mean = cv::Scalar(0, 0, 0);
    bool  letterbox = true;
    float confThreshold = 0.25f;
    float nmsThreshold = 0.45f;
    // �ԋp����LabelObj�̕`��F��Ă͔j���ŕ\���itrue�����j
    bool  proposalDashed = true;
};

// �֐��S�̂̃p�����[�^
struct DnnParams {
    DnnOptions opt;
    YoloConfig yolo;
};

///////////////////////////////////////////////////////////////////////////////////////////
// �摜(Mat)��ONNX�t�@�C������A���_�ς�LabelObj�z���Ԃ��i���K��xywh�j
///////////////////////////////////////////////////////////////////////////////////////////
std::vector<LabelObj> DnnInfer(const cv::Mat& bgr,
    const std::wstring& onnxPath,
    const DnnParams& params = DnnParams());

///////////////////////////////////////////////////////////////////////////////////////////
// �\���p���[�e�B���e�B�֐�
///////////////////////////////////////////////////////////////////////////////////////////
RectF FitImageToClientRect(int imgW, int imgH, const RECT& rcClient);
RectF NormToViewRect(const RectF& rNorm, const RectF& view);
void DrawLabelObjects(Graphics& g, const std::vector<LabelObj>& objs, const RectF& view);


//extern std::vector<LabelObj> AutoDetctedObjs; // ���_�̈ꎞ�\���p
extern ImgObject AutoDetctedObjs; // ���_�̈ꎞ�\���p

extern bool g_showProposals;    // = true;       // �\��ON/OFF
//extern DnnParams GDNNP;


///////////////////////////////////////////////////////////////////////////////////////////
// ���[�e�B���e�B�֐�: std::wstring��std::string�ɕϊ�
///////////////////////////////////////////////////////////////////////////////////////////
inline std::string WStringToString(const std::wstring& wstr)
{
    std::string str(wstr.begin(), wstr.end());
    return str;
}

inline RectF ToRectF(const RECT& rc)
{
    return RectF(
        static_cast<REAL>(rc.left),
        static_cast<REAL>(rc.top),
        static_cast<REAL>(rc.right - rc.left),
        static_cast<REAL>(rc.bottom - rc.top)
    );
}