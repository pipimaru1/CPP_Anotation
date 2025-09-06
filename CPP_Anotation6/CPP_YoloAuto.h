#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric> // Add this include at the top of the file

#include "CPP_AnnoGblParams.h" // LabelObj, GlobalParams(GP)

// 推論バックエンド/ターゲット
struct DnnOptions {
    int backend = cv::dnn::DNN_BACKEND_OPENCV; // CUDA可: DNN_BACKEND_CUDA
    int target = cv::dnn::DNN_TARGET_CPU;     // CUDA可: DNN_TARGET_CUDA
    bool reuse_net = true;                     // ONNXをキャッシュ
};

// YOLO前後処理パラメータ
struct YoloConfig {
    int   inputW = 640;
    int   inputH = 640;
    bool  swapRB = true;
    float scale = 1.0f / 255.0f;
    cv::Scalar mean = cv::Scalar(0, 0, 0);
    bool  letterbox = true;

    //自動アノテーション 最高感度にしておく 超好感度はメニューで設定
    float confThreshold = 0.1f;
    float nmsThreshold = 0.30f;
    // 返却するLabelObjの描画：提案は破線で表示（true推奨）
    bool  proposalDashed = true;

    int   numClasses = -1;      // 未指定なら自動推定
    bool  hasObjness = false;   // v5/v7ならtrue、v8ならfalse。未確定なら numClasses=-1 で自動に任せる
    bool  applySigmoid = false; // ONNXにSigmoidが残っていなければ true
    int   YoloVersion = 5;      // 5, 6, 7, 8
};

// 関数全体のパラメータ
struct DnnParams {
    DnnOptions opt;
    YoloConfig yolo;
    std::wstring gOnnxPath = L".\yolov5s.onnx"; // ONNXファイルパス

    cv::dnn::Net net;
    std::wstring PregOnnxPath;
};

///////////////////////////////////////////////////////////////////////////////////////////
// 画像(Mat)とONNXファイルから、推論済みLabelObj配列を返す（正規化xywh）
///////////////////////////////////////////////////////////////////////////////////////////
std::vector<LabelObj> DnnInfer(
    int yolo_version,
    const cv::Mat& bgr,
    const std::wstring& onnxPath,
    const DnnParams& params = DnnParams());

///////////////////////////////////////////////////////////////////////////////////////////
// 表示用ユーティリティ関数
///////////////////////////////////////////////////////////////////////////////////////////
RectF FitImageToClientRect(int imgW, int imgH, const RECT& rcClient);
RectF NormToViewRect(const RectF& rNorm, const RectF& view);
void DrawLabelObjects(Graphics& g, const std::vector<LabelObj>& objs, const RectF& view, 
    //Gdiplus::Color _color = Gdiplus::Color::LightGray, 
    Gdiplus::Color _color = Gdiplus::Color::White,
    int _penwidth = 2);


///////////////////////////////////////////////////////////////////////////////////////////
// ユーティリティ関数: std::wstringをstd::stringに変換
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
// グローバル変数
///////////////////////////////////////////////////////////////////////////////////////////
//extern std::vector<LabelObj> AutoDetctedObjs; // 推論の一時表示用
extern ImgObject AutoDetctedObjs; // 推論の一時表示用
extern bool g_showProposals;    // = true;       // 表示ON/OFF
//extern DnnParams GDNNP;
//extern std::wstring g_onnxFile; // ONNXファイルパス
extern DnnParams GDNNP; // DNNパラメータ のグローバル変数
