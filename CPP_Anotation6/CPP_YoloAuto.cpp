#include "pch.h"
#include "CPP_YoloAuto.h"

#include <mutex>
#include <unordered_map>
#ifdef _WIN32
#include <windows.h>
#endif

//�\���p
#include <gdiplus.h>
using namespace Gdiplus;

///////////////////////////////////////////////////////////////////////////////////////////
// �O���[�o���ϐ�
///////////////////////////////////////////////////////////////////////////////////////////
ImgObject AutoDetctedObjs; // ���_�̈ꎞ�\���p
bool g_showProposals = false;       // �\��ON/OFF
//std::wstring g_onnxFile = L""; // ONNX�t�@�C���p�X

DnnParams GDNNP;

///////////////////////////////////////////////////////////////////////////////////////////
// �֐�
///////////////////////////////////////////////////////////////////////////////////////////
static std::string WToUtf8(const std::wstring& w)
{
#ifdef _WIN32
    if (w.empty()) return std::string();
    int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], len, nullptr, nullptr);
    return s;
#else
    return std::string(w.begin(), w.end()); // �Ȉ�
#endif
}

static cv::dnn::Net LoadOrGetNet(const std::wstring& onnxPath, const DnnOptions& opt)
{
    static std::mutex mtx;
    static std::unordered_map<std::wstring, cv::dnn::Net> cache;

    if (opt.reuse_net) {
        std::lock_guard<std::mutex> lk(mtx);
        auto it = cache.find(onnxPath);
        if (it != cache.end()) return it->second;
        cv::dnn::Net net = cv::dnn::readNetFromONNX(WToUtf8(onnxPath));
        net.setPreferableBackend(opt.backend);
        net.setPreferableTarget(opt.target);
        cache.emplace(onnxPath, net);
        return net;
    }
    else {
        cv::dnn::Net net = cv::dnn::readNetFromONNX(WToUtf8(onnxPath));
        net.setPreferableBackend(opt.backend);
        net.setPreferableTarget(opt.target);
        return net;
    }
}

// ���^�[�{�b�N�X�t���őO����
static void MakeBlobResizeLetterbox(const cv::Mat& bgr, const YoloConfig& yc,
    cv::Mat& blob, cv::Rect& padRect)
{
    cv::Mat resized;
    if (yc.letterbox) {
        float r = std::min(yc.inputW / (float)bgr.cols, yc.inputH / (float)bgr.rows);
        int nw = (int)std::round(bgr.cols * r);
        int nh = (int)std::round(bgr.rows * r);
        cv::resize(bgr, resized, cv::Size(nw, nh));
        int dw = yc.inputW - nw;
        int dh = yc.inputH - nh;
        padRect = cv::Rect(dw / 2, dh / 2, nw, nh);
        cv::copyMakeBorder(resized, resized, dh / 2, dh - dh / 2, dw / 2, dw - dw / 2,
            cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    }
    else {
        cv::resize(bgr, resized, cv::Size(yc.inputW, yc.inputH));
        padRect = cv::Rect(0, 0, yc.inputW, yc.inputH);
    }
    blob = cv::dnn::blobFromImage(resized, yc.scale, cv::Size(yc.inputW, yc.inputH),
        yc.mean, yc.swapRB, false);
}

// YOLO(v5/v8) �ėp�f�R�[�h�iforward�o�͂�(1,N,85) or (1,85,N) or (1,84,N)�z��j
inline float Sigmoid(float x) { return 1.f / (1.f + std::exp(-x)); }


static void DecodeYoloGeneric(const cv::Mat& out, const YoloConfig& yc,
    const cv::Size& orig, const cv::Rect& padRect,
    std::vector<cv::Rect>& boxesPx,
    std::vector<float>& scores,
    std::vector<int>& classIds)
{
    CV_Assert(out.dims == 3);
    int A = out.size[1], B = out.size[2];
    bool channels_first = (A == 84 || A == 85 || A == 116);
    int C = channels_first ? A : B; // ������
    int N = channels_first ? B : A; // ��␔
    const float* p = (const float*)out.data;

    float gain = std::min(yc.inputW / (float)orig.width, yc.inputH / (float)orig.height);
    float padX = (float)padRect.x, padY = (float)padRect.y;

    std::vector<cv::Rect>  tmpBoxes; tmpBoxes.reserve(N);
    std::vector<float>     tmpScores; tmpScores.reserve(N);
    std::vector<int>       tmpCls; tmpCls.reserve(N);

    for (int i = 0; i < N; ++i) {
        float cx = channels_first ? p[0 * N + i] : p[i * C + 0];
        float cy = channels_first ? p[1 * N + i] : p[i * C + 1];
        float w = channels_first ? p[2 * N + i] : p[i * C + 2];
        float h = channels_first ? p[3 * N + i] : p[i * C + 3];

        float obj = 1.0f; int clsStart = 4;
        if (C >= 85) { obj = channels_first ? p[4 * N + i] : p[i * C + 4]; clsStart = 5; }

        int bestId = -1; float bestScore = -1.0f;
        for (int c = clsStart; c < C; ++c) {
            float s = channels_first ? p[c * N + i] : p[i * C + c];
            float conf = s * obj;
            if (conf > bestScore) { bestScore = conf; bestId = c - clsStart; }
        }
        if (bestScore < yc.confThreshold) continue;

        float x = cx - w * 0.5f, y = cy - h * 0.5f;
        float x0 = (x - padX) / gain;
        float y0 = (y - padY) / gain;
        float x1 = (x + w - padX) / gain;
        float y1 = (y + h - padY) / gain;

        x0 = std::max(0.f, std::min(x0, (float)orig.width - 1));
        y0 = std::max(0.f, std::min(y0, (float)orig.height - 1));
        x1 = std::max(0.f, std::min(x1, (float)orig.width - 1));
        y1 = std::max(0.f, std::min(y1, (float)orig.height - 1));

        cv::Rect box((int)std::round(x0), (int)std::round(y0),
            (int)std::round(x1 - x0), (int)std::round(y1 - y0));
        if (box.width <= 0 || box.height <= 0) continue;

        tmpBoxes.push_back(box);
        tmpScores.push_back(bestScore);
        tmpCls.push_back(bestId);
    }

    std::vector<int> idx;
    cv::dnn::NMSBoxes(tmpBoxes, tmpScores, yc.confThreshold, yc.nmsThreshold, idx);

    boxesPx.reserve(idx.size()); scores.reserve(idx.size()); classIds.reserve(idx.size());
    for (int k : idx) {
        boxesPx.push_back(tmpBoxes[k]);
        scores.push_back(tmpScores[k]);
        classIds.push_back(tmpCls[k]);
    }
}
/*
static void DecodeYoloGeneric(const cv::Mat& out, const YoloConfig& yc,
    const cv::Size& orig, const cv::Rect& padRect,
    std::vector<cv::Rect>& boxesPx,
    std::vector<float>& scores,
    std::vector<int>& classIds)
{
    CV_Assert(out.dims == 3);
    const int A = out.size[1], B = out.size[2];

    // [C,N] / [N,C] ����
    const bool channels_first = (A < B);
    const int  C = channels_first ? A : B;
    const int  N = channels_first ? B : A;
    const float* p = (const float*)out.data;

    auto idx = [&](int attr, int i)->int {
        return channels_first ? (attr * N + i) : (i * C + attr);
        };

    // ==== v8�D��� C=4+nc �� ���� v5/7 �� C=5+nc �𔻒�i���Ԃ��d�v�j ====
    int  nc = (yc.numClasses > 0) ? yc.numClasses : -1;
    bool hasObj = yc.hasObjness;
    if (nc <= 0) {
        if (C - 4 >= 1 && C - 4 <= 2048) { nc = C - 4; hasObj = false; } // v8�n
        else if (C - 5 >= 1 && C - 5 <= 2048) { nc = C - 5; hasObj = true; } // v5/7�n
        else { return; }
    }
    const int clsStart = hasObj ? 5 : 4;

    // ==== �o�͂��m�������W�b�g�����y����������i�K�v�Ȃ� Sigmoid�j ====
    auto peek_minmax = [&](int start, int count)->std::pair<float, float> {
        float mn = FLT_MAX, mx = -FLT_MAX;
        const int stride = std::max(1, N / 64);
        for (int i = 0, t = 0; i < N && t < 64; i += stride, ++t) {
            for (int c = 0; c < count; ++c) {
                float v = p[idx(start + c, i)];
                mn = std::min(mn, v); mx = std::max(mx, v);
            }
        }
        return { mn, mx };
        };
    auto [mnCls, mxCls] = peek_minmax(clsStart, std::min(nc, 4));
    bool needSigmoidCls = (mnCls < -1e-3f) || (mxCls > 1.f + 1e-3f);

    bool needSigmoidObj = false;
    if (hasObj) {
        auto [mnObj, mxObj] = peek_minmax(4, 1);
        needSigmoidObj = (mnObj < -1e-3f) || (mxObj > 1.f + 1e-3f);
    }
    // ���[�U�ݒ�Ŗ����������ꍇ�� yc.applySigmoid �� OR ���Ă�������
    needSigmoidCls = needSigmoidCls || yc.applySigmoid;
    needSigmoidObj = needSigmoidObj || (yc.applySigmoid && hasObj);

    // ==== ���W�����ɕK�v�ȃX�P�[�����O ====
    const float gain = std::min(yc.inputW / (float)orig.width, yc.inputH / (float)orig.height);
    const float padX = (float)padRect.x, padY = (float)padRect.y;

    // ==== �܂��͌��𗭂߂� ====
    std::vector<cv::Rect>  tmpBoxes;  tmpBoxes.reserve(N);
    std::vector<float>     tmpScores; tmpScores.reserve(N);
    std::vector<int>       tmpCls;    tmpCls.reserve(N);

    for (int i = 0; i < N; ++i) {
        float cx = p[idx(0, i)];
        float cy = p[idx(1, i)];
        float w = p[idx(2, i)];
        float h = p[idx(3, i)];

        float obj = 1.f;
        if (hasObj) {
            obj = p[idx(4, i)];
            if (needSigmoidObj) obj = Sigmoid(obj);
        }

        int   bestId = -1;
        float bestScore = -1.f;
        for (int c = 0; c < nc; ++c) {
            float s = p[idx(clsStart + c, i)];
            if (needSigmoidCls) s = Sigmoid(s);
            float conf = hasObj ? (s * obj) : s; // v8�n�� obj �Ȃ�
            if (conf > bestScore) { bestScore = conf; bestId = c; }
        }
        if (bestScore < yc.confThreshold) continue;

        // xywh(center) -> xyxy -> ���摜���W�i���^�[�{�b�N�X�����j
        float x0 = (cx - w * 0.5f - padX) / gain;
        float y0 = (cy - h * 0.5f - padY) / gain;
        float x1 = (cx + w * 0.5f - padX) / gain;
        float y1 = (cy + h * 0.5f - padY) / gain;

        x0 = std::max(0.f, std::min(x0, (float)orig.width - 1));
        y0 = std::max(0.f, std::min(y0, (float)orig.height - 1));
        x1 = std::max(0.f, std::min(x1, (float)orig.width - 1));
        y1 = std::max(0.f, std::min(y1, (float)orig.height - 1));

        const int ix0 = (int)std::round(x0), iy0 = (int)std::round(y0);
        const int iw = (int)std::round(x1 - x0), ih = (int)std::round(y1 - y0);
        if (iw <= 0 || ih <= 0) continue;

        tmpBoxes.emplace_back(ix0, iy0, iw, ih);
        tmpScores.emplace_back(bestScore);
        tmpCls.emplace_back(bestId);
    }

    // ==== NMS�i�������Ɠ����A�N���X���f�� NMS�j====
    std::vector<int> idx2;
    cv::dnn::NMSBoxes(tmpBoxes, tmpScores, yc.confThreshold, yc.nmsThreshold, idx2);


    boxesPx.clear(); scores.clear(); classIds.clear();
    boxesPx.reserve(idx2.size()); scores.reserve(idx2.size()); classIds.reserve(idx2.size());
    for (int k : idx2) {
        boxesPx.push_back(tmpBoxes[k]);
        scores.push_back(tmpScores[k]);
        classIds.push_back(tmpCls[k]);
    }
}
*/

//static void DecodeYoloGeneric(const cv::Mat& out, const YoloConfig& yc,
//    const cv::Size& orig, const cv::Rect& padRect,
//    std::vector<cv::Rect>& boxesPx,
//    std::vector<float>& scores,
//    std::vector<int>& classIds)
//{
//    CV_Assert(out.dims == 3);
//    const int A = out.size[1];
//    const int B = out.size[2];
//
//    // �������͒ʏ�u��␔�v��菬�����̂ŁAA<B �Ȃ� [C(����), N(���)] �Ƃ݂Ȃ�
//    const bool channels_first = (A < B);
//    const int  C = channels_first ? A : B; // ������ = 4(+1) + nc
//    const int  N = channels_first ? B : A; // ��␔
//
//    const float* p = (const float*)out.data;
//
//    // ---- �N���X�� / objness �̎�������i�����w�肪����ΗD��j----
//    int  nc = (yc.numClasses > 0) ? yc.numClasses : -1;
//    bool hasObj = yc.hasObjness;           // -1 �I�ȁu�s���v�t���O���Ȃ���΁Afalse/true�̂ǂ��炩�����Ă���
//
//    if (nc <= 0) {
//        // C �� 4+nc�iv8�n: objness�����j �܂��� 5+nc�iv5/7�n: objness�L��j�ł��邱�Ƃ�����
//        if (C - 5 >= 1 && C - 5 <= 1024) { nc = C - 5; hasObj = true; }
//        else if (C - 4 >= 1 && C - 4 <= 1024) { nc = C - 4; hasObj = false; }
//        else {
//            // �z��O�`��i�i�X���j�����S�ɏI��
//            return;
//        }
//    }
//    const int clsStart = hasObj ? 5 : 4;
//
//    const bool doSigmoid = yc.applySigmoid; // ONNX��Sigmoid���c���Ă����false, ���W�b�g�Ȃ�true
//
//    const float gain = std::min(yc.inputW / (float)orig.width, yc.inputH / (float)orig.height);
//    const float padX = (float)padRect.x, padY = (float)padRect.y;
//
//    boxesPx.clear(); scores.clear(); classIds.clear();
//    boxesPx.reserve(N); scores.reserve(N); classIds.reserve(N);
//
//    auto idx = [&](int attr, int i)->int {
//        // channels_first: [C, N] �Ȃ̂� attr*N + i
//        // channels_last : [N, C] �Ȃ̂� i*C + attr
//        return channels_first ? (attr * N + i) : (i * C + attr);
//        };
//
//    for (int i = 0; i < N; ++i) {
//        float cx = p[idx(0, i)];
//        float cy = p[idx(1, i)];
//        float w = p[idx(2, i)];
//        float h = p[idx(3, i)];
//
//        float obj = 1.f;
//        if (hasObj) {
//            obj = p[idx(4, i)];
//            if (doSigmoid) obj = Sigmoid(obj);
//        }
//
//        int   bestId = -1;
//        float bestScore = -1.f;
//
//        for (int c = 0; c < nc; ++c) {
//            float s = p[idx(clsStart + c, i)];
//            if (doSigmoid) s = Sigmoid(s);
//            const float conf = s * obj;  // v8�n�iobj�Ȃ��j�̏ꍇ�� obj=1 �Ȃ̂ł��̂܂�
//            if (conf > bestScore) { bestScore = conf; bestId = c; }
//        }
//        if (bestScore < yc.confThreshold) continue;
//
//        // xywh(center) -> xyxy, letterbox�������Č��摜���W��
//        float x = cx - w * 0.5f, y = cy - h * 0.5f;
//        float x0 = (x - padX) / gain;
//        float y0 = (y - padY) / gain;
//        float x1 = (x + w - padX) / gain;
//        float y1 = (y + h - padY) / gain;
//
//        x0 = std::max(0.f, std::min(x0, (float)orig.width - 1));
//        y0 = std::max(0.f, std::min(y0, (float)orig.height - 1));
//        x1 = std::max(0.f, std::min(x1, (float)orig.width - 1));
//        y1 = std::max(0.f, std::min(y1, (float)orig.height - 1));
//
//        const int ix0 = (int)std::round(x0), iy0 = (int)std::round(y0);
//        const int iw = (int)std::round(x1 - x0), ih = (int)std::round(y1 - y0);
//        if (iw <= 0 || ih <= 0) continue;
//
//        boxesPx.push_back(cv::Rect(ix0, iy0, iw, ih));
//        scores.push_back(bestScore);
//        classIds.push_back(bestId);
//    }
//}

static inline Gdiplus::RectF PxToNormXYWH_RectF(const cv::Rect& r, const cv::Size& sz)
{
    return Gdiplus::RectF(
        r.x / (float)sz.width,
        r.y / (float)sz.height,
        r.width / (float)sz.width,
        r.height / (float)sz.height
    );
}

static inline void SetupStyleForProposal(LabelObj& L, int classId, const YoloConfig& yc)
{
    // �����̃N���X�ݒ�𓥏P�i�F�E����E�����j
    if (classId >= 0 && classId < (int)GP.ClsColors.size())      L.color = GP.ClsColors[classId];
    if (classId >= 0 && classId < (int)GP.ClsDashStyles.size())  L.dashStyle = GP.ClsDashStyles[classId];
    if (classId >= 0 && classId < (int)GP.ClsPenWidths.size())   L.penWidth = GP.ClsPenWidths[classId];

    // ��Ă͔j���Ō����������ꍇ
    if (yc.proposalDashed) L.dashStyle = Gdiplus::DashStyleDash;
    if (L.penWidth <= 0)   L.penWidth = 2;
}

//inline void NMS(
//    const DnnParams& params,
//    std::vector<cv::Rect>& boxesPx,
//    std::vector<int>& classIds,
//    std::vector<float>& scores
//)
//{
//    // ---- �������� NMS �ǉ� ----
//    std::vector<int> keep;
//    keep.reserve(boxesPx.size());
//
//    const float score_th = params.yolo.confThreshold; // ��: 0.25
//    const float nms_th = params.yolo.nmsThreshold;  // ��: 0.45
//
//    // �@�N���X��NMS�i�ʏ�͂�����j
//    {
//        // �N���XID�̍ő�l����N���X���𐄒�inumClasses ���ݒ�ς݂Ȃ炻����g�p�j
//        int nc = params.yolo.numClasses > 0 ? params.yolo.numClasses
//            : (classIds.empty() ? 0 : (*std::max_element(classIds.begin(), classIds.end()) + 1));
//
//        for (int c = 0; c < nc; ++c) {
//            std::vector<cv::Rect>  Bc;
//            std::vector<float>     Sc;
//            std::vector<int>       Ic;   // ���C���f�b�N�X
//
//            for (int i = 0; i < (int)boxesPx.size(); ++i) {
//                if (classIds[i] == c) {
//                    Bc.push_back(boxesPx[i]);
//                    Sc.push_back(scores[i]);
//                    Ic.push_back(i);
//                }
//            }
//            if (Bc.empty()) continue;
//
//            std::vector<int> idx;
//            cv::dnn::NMSBoxes(Bc, Sc, score_th, nms_th, idx);
//            for (int j : idx) keep.push_back(Ic[j]);
//        }
//    }
//}

std::vector<LabelObj> DnnInfer(const cv::Mat& bgr,
    const std::wstring& onnxPath,
    const DnnParams& params)
{
    std::vector<LabelObj> outLabels;
    
    // ONNX�t�@�C���������Ȃ牽�����Ȃ�
	if (onnxPath.empty()) 
        return std::vector<LabelObj>();
    else
    {
        // �摜�������Ȃ牽�����Ȃ�
        if (bgr.empty())
            return outLabels;
        else
        {
            cv::dnn::Net net = LoadOrGetNet(onnxPath, params.opt);
            // �O����
            cv::Mat blob; cv::Rect padRect;
            MakeBlobResizeLetterbox(bgr, params.yolo, blob, padRect);
            net.setInput(blob);
            cv::Mat out;

            // ���_ ���s�������ŕԂ�
            try
            {
                out = net.forward();
            }
            catch (cv::Exception& e)
            {
                // ��O���L���b�`������G���[���b�Z�[�W��\��
                std::cerr << e.what() << std::endl;
                std::wstring err = L"ONNX���f���̐��_�Ɏ��s���܂����B\n���f�������Ă��邩�A\n���f���̌`�����Ή����Ă��Ȃ��\��������܂��B\n";
                err += std::wstring(e.what(), e.what() + strlen(e.what()));
                MessageBoxExW(NULL, err.c_str(), L"Error", MB_OK | MB_ICONERROR, MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN));
                return outLabels;
            }

            // �㏈���iYOLO�ėp�f�R�[�h�j
            std::vector<cv::Rect> boxesPx;
            std::vector<float>    scores;
            std::vector<int>      classIds;
            DecodeYoloGeneric(out, params.yolo, bgr.size(), padRect, boxesPx, scores, classIds);

            //NMS(params, boxesPx, classIds, scores);

            // LabelObj�ɕϊ��i���K��xywh�j
            outLabels.reserve(boxesPx.size());
            for (size_t i = 0; i < boxesPx.size(); ++i) {
                LabelObj L;
                L.Rct = PxToNormXYWH_RectF(boxesPx[i], bgr.size());

                //L.Rct_Scale = L.rect; // �K�v�Ȃ�`�掞�ɍČv�Z

                L.ClassNum = classIds[i];
                if (L.ClassNum >= 0 && L.ClassNum < (int)GP.ClsNames.size())
                    L.ClassName = GP.ClsNames[L.ClassNum];
                SetupStyleForProposal(L, L.ClassNum, params.yolo);
                outLabels.push_back(L);
            }
        }
    }
    return outLabels;
}


///////////////////////////////////////////////////////////////////////////////////////////
// �E�B���h�E�S�̂ɉ摜���t�B�b�g������\����`���v�Z
///////////////////////////////////////////////////////////////////////////////////////////
RectF FitImageToClientRect(int imgW, int imgH, const RECT& rcClient)
{
    const float cw = (float)(rcClient.right - rcClient.left);
    const float ch = (float)(rcClient.bottom - rcClient.top);
    const float sx = cw / imgW, sy = ch / imgH, s = (sx < sy) ? sx : sy;
    const float w = imgW * s, h = imgH * s;
    const float x = rcClient.left + (cw - w) * 0.5f;
    const float y = rcClient.top + (ch - h) * 0.5f;
    return RectF(x, y, w, h);
}

///////////////////////////////////////////////////////////////////////////////////////////
// ���K��xywh �� �\����`��
///////////////////////////////////////////////////////////////////////////////////////////
RectF NormToViewRect(const RectF& rNorm, const RectF& view)
{
    return RectF(
        view.X + rNorm.X * view.Width,
        view.Y + rNorm.Y * view.Height,
        rNorm.Width * view.Width,
        rNorm.Height * view.Height
    );
}


///////////////////////////////////////////////////////////////////////////////////////////
// LabelObj�z���`��i�j���E�F�E������LabelObj�̒l���g�p�j
///////////////////////////////////////////////////////////////////////////////////////////
void DrawLabelObjects(Graphics& g, const std::vector<LabelObj>& objs, const RectF& view, Gdiplus::Color _color, int _penwidth )
{
    for (const auto& L : objs) {
        RectF r = NormToViewRect(L.Rct, view);

        //Pen pen(L.color, (REAL)(L.penWidth > 0 ? L.penWidth : 2));
        Pen pen(_color, (REAL)(_penwidth > 0 ? _penwidth : 2));

        pen.SetDashStyle(L.dashStyle);                         // �j���Łu��āv��
        g.DrawRectangle(&pen, r.X, r.Y, r.Width, r.Height);

        if (!L.ClassName.empty()) {
            //SolidBrush br(Color(200, L.color.GetR(), L.color.GetG(), L.color.GetB())); // ������
            SolidBrush br(Color(200, _color.GetR(), _color.GetG(), _color.GetB())); // ������

            Font font(L"Segoe UI", 12, FontStyleBold, UnitPixel);
            g.DrawString(L.ClassName.c_str(), -1, &font, PointF(r.X + 2, r.Y + 2), &br);
        }
    }
}



