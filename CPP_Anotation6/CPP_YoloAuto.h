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
    DNN_KIND_AUTO = 0,   // 出力形状から自動推定（YOLO検出 or 画像分類）
    DNN_KIND_YOLO,       // YOLO系検出（v5/v7/v8 など）
    DNN_KIND_CLS         // 画像分類（1xC or 1xCx1x1 ロジット）
};

enum BoxFormat {
    BOX_XYWH_NORM = 0,   // 正規化xywh（左上起点・[0..1]）
    BOX_XYWH_PIXEL,      // ピクセルxywh（左上起点）
    BOX_XYXY_PIXEL       // ピクセルxyxy（minmax）
};

struct DnnOptions {
    int backend = cv::dnn::DNN_BACKEND_OPENCV; // CUDA可: DNN_BACKEND_CUDA
    int target = cv::dnn::DNN_TARGET_CPU;     // CUDA可: DNN_TARGET_CUDA
    bool reuse_net = true;                     // ONNXをキャッシュして使い回し
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
    int   topk = 5; // 上位k件
};

struct DetBox {
    int   classId;
    float conf;
    cv::Rect2f box; // boxFormatに従う（デフォルト: 正規化xywh）
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
    std::vector<DetBox> dets; // 検出があれば格納
    std::vector<ClsScore> cls;// 分類があれば格納（top-k）
    double preprocessMs = 0.0;
    double inferMs = 0.0;
    double postMs = 0.0;
};

// これ1本でOK：画像Mat、ONNXパス、各種パラメータ → 検出/分類の配列を返す
DnnResult DnnInfer(const cv::Mat& bgr,
    const std::wstring& onnxPath,
    const DnnParams& params = DnnParams());
*/

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
    float confThreshold = 0.25f;
    float nmsThreshold = 0.45f;
    // 返却するLabelObjの描画：提案は破線で表示（true推奨）
    bool  proposalDashed = true;
};

// 関数全体のパラメータ
struct DnnParams {
    DnnOptions opt;
    YoloConfig yolo;
};

///////////////////////////////////////////////////////////////////////////////////////////
// 画像(Mat)とONNXファイルから、推論済みLabelObj配列を返す（正規化xywh）
///////////////////////////////////////////////////////////////////////////////////////////
std::vector<LabelObj> DnnInfer(const cv::Mat& bgr,
    const std::wstring& onnxPath,
    const DnnParams& params = DnnParams());

///////////////////////////////////////////////////////////////////////////////////////////
// 表示用ユーティリティ関数
///////////////////////////////////////////////////////////////////////////////////////////
RectF FitImageToClientRect(int imgW, int imgH, const RECT& rcClient);
RectF NormToViewRect(const RectF& rNorm, const RectF& view);
void DrawLabelObjects(Graphics& g, const std::vector<LabelObj>& objs, const RectF& view);


//extern std::vector<LabelObj> AutoDetctedObjs; // 推論の一時表示用
extern ImgObject AutoDetctedObjs; // 推論の一時表示用

extern bool g_showProposals;    // = true;       // 表示ON/OFF
//extern DnnParams GDNNP;


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