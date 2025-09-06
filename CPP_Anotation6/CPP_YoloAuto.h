#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric> // Add this include at the top of the file

#include "CPP_AnnoGblParams.h" // LabelObj, GlobalParams(GP)

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

    //�����A�m�e�[�V���� �ō����x�ɂ��Ă��� ���D���x�̓��j���[�Őݒ�
    float confThreshold = 0.1f;
    float nmsThreshold = 0.30f;
    // �ԋp����LabelObj�̕`��F��Ă͔j���ŕ\���itrue�����j
    bool  proposalDashed = true;

    int   numClasses = -1;      // ���w��Ȃ玩������
    bool  hasObjness = false;   // v5/v7�Ȃ�true�Av8�Ȃ�false�B���m��Ȃ� numClasses=-1 �Ŏ����ɔC����
    bool  applySigmoid = false; // ONNX��Sigmoid���c���Ă��Ȃ���� true
    int   YoloVersion = 5;      // 5, 6, 7, 8
};

// �֐��S�̂̃p�����[�^
struct DnnParams {
    DnnOptions opt;
    YoloConfig yolo;
    std::wstring gOnnxPath = L".\yolov5s.onnx"; // ONNX�t�@�C���p�X

    cv::dnn::Net net;
    std::wstring PregOnnxPath;
};

///////////////////////////////////////////////////////////////////////////////////////////
// �摜(Mat)��ONNX�t�@�C������A���_�ς�LabelObj�z���Ԃ��i���K��xywh�j
///////////////////////////////////////////////////////////////////////////////////////////
std::vector<LabelObj> DnnInfer(
    int yolo_version,
    const cv::Mat& bgr,
    const std::wstring& onnxPath,
    const DnnParams& params = DnnParams());

///////////////////////////////////////////////////////////////////////////////////////////
// �\���p���[�e�B���e�B�֐�
///////////////////////////////////////////////////////////////////////////////////////////
RectF FitImageToClientRect(int imgW, int imgH, const RECT& rcClient);
RectF NormToViewRect(const RectF& rNorm, const RectF& view);
void DrawLabelObjects(Graphics& g, const std::vector<LabelObj>& objs, const RectF& view, 
    //Gdiplus::Color _color = Gdiplus::Color::LightGray, 
    Gdiplus::Color _color = Gdiplus::Color::White,
    int _penwidth = 2);


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

///////////////////////////////////////////////////////////////////////////////////////////
// �O���[�o���ϐ�
///////////////////////////////////////////////////////////////////////////////////////////
//extern std::vector<LabelObj> AutoDetctedObjs; // ���_�̈ꎞ�\���p
extern ImgObject AutoDetctedObjs; // ���_�̈ꎞ�\���p
extern bool g_showProposals;    // = true;       // �\��ON/OFF
//extern DnnParams GDNNP;
//extern std::wstring g_onnxFile; // ONNX�t�@�C���p�X
extern DnnParams GDNNP; // DNN�p�����[�^ �̃O���[�o���ϐ�
