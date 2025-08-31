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

//std::vector<LabelObj> AutoDetctedObjs; // ���_�̈ꎞ�\���p
ImgObject AutoDetctedObjs; // ���_�̈ꎞ�\���p
bool g_showProposals = false;       // �\��ON/OFF

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

std::vector<LabelObj> DnnInfer(const cv::Mat& bgr,
    const std::wstring& onnxPath,
    const DnnParams& params)
{
    std::vector<LabelObj> outLabels;
    if (bgr.empty()) return outLabels;

    cv::dnn::Net net = LoadOrGetNet(onnxPath, params.opt);

    // �O����
    cv::Mat blob; cv::Rect padRect;
    MakeBlobResizeLetterbox(bgr, params.yolo, blob, padRect);
    net.setInput(blob);

    // ���_
    cv::Mat out = net.forward();

    // �㏈���iYOLO�ėp�f�R�[�h�j
    std::vector<cv::Rect> boxesPx;
    std::vector<float>    scores;
    std::vector<int>      classIds;
    DecodeYoloGeneric(out, params.yolo, bgr.size(), padRect, boxesPx, scores, classIds);

    // LabelObj�ɕϊ��i���K��xywh�j
    outLabels.reserve(boxesPx.size());
    for (size_t i = 0; i < boxesPx.size(); ++i) {
        LabelObj L;
        L.rect = PxToNormXYWH_RectF(boxesPx[i], bgr.size());
        
        //L.Rct_Scale = L.rect; // �K�v�Ȃ�`�掞�ɍČv�Z

        L.ClassNum = classIds[i];
        if (L.ClassNum >= 0 && L.ClassNum < (int)GP.ClsNames.size())
            L.ClassName = GP.ClsNames[L.ClassNum];
        SetupStyleForProposal(L, L.ClassNum, params.yolo);
        outLabels.push_back(L);
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
void DrawLabelObjects(Graphics& g, const std::vector<LabelObj>& objs, const RectF& view)
{
    for (const auto& L : objs) {
        RectF r = NormToViewRect(L.rect, view);

        Pen pen(L.color, (REAL)(L.penWidth > 0 ? L.penWidth : 2));
        pen.SetDashStyle(L.dashStyle);                         // �j���Łu��āv��
        g.DrawRectangle(&pen, r.X, r.Y, r.Width, r.Height);

        if (!L.ClassName.empty()) {
            SolidBrush br(Color(200, L.color.GetR(), L.color.GetG(), L.color.GetB())); // ������
            Font font(L"Segoe UI", 12, FontStyleBold, UnitPixel);
            g.DrawString(L.ClassName.c_str(), -1, &font, PointF(r.X + 2, r.Y + 2), &br);
        }
    }
}


/*
static cv::dnn::Net LoadOrGetNet(const std::wstring& onnxPath, const DnnOptions& opt) {
    static std::mutex mtx;
    static std::unordered_map<std::wstring, cv::dnn::Net> cache;

    if (opt.reuse_net) {
        std::lock_guard<std::mutex> lk(mtx);
        auto it = cache.find(onnxPath);
        if (it != cache.end()) return it->second;
        cv::dnn::Net net = cv::dnn::readNetFromONNX(std::string(onnxPath.begin(), onnxPath.end()));
        net.setPreferableBackend(opt.backend);
        net.setPreferableTarget(opt.target);
        cache.emplace(onnxPath, net);
        return net;
    }
    else {
        cv::dnn::Net net = cv::dnn::readNetFromONNX(std::string(onnxPath.begin(), onnxPath.end()));
        net.setPreferableBackend(opt.backend);
        net.setPreferableTarget(opt.target);
        return net;
    }
}

static void MakeBlobResizeLetterbox(const cv::Mat& bgr, const YoloConfig& yc,
    cv::Mat& blob, cv::Size& resizedSz, cv::Rect& padRect)
{
    cv::Mat resized;
    if (yc.letterbox) {
        float r = std::min(yc.inputW / (float)bgr.cols, yc.inputH / (float)bgr.rows);
        int nw = (int)std::round(bgr.cols * r);
        int nh = (int)std::round(bgr.rows * r);
        cv::resize(bgr, resized, cv::Size(nw, nh));
        int dw = yc.inputW - nw;
        int dh = yc.inputH - nh;
        padRect = cv::Rect(dw / 2, dh / 2, nw, nh); // �摜���\��t����`�i���͍��W�n�j
        cv::copyMakeBorder(resized, resized, dh / 2, dh - dh / 2, dw / 2, dw - dw / 2,
            cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    }
    else {
        cv::resize(bgr, resized, cv::Size(yc.inputW, yc.inputH));
        padRect = cv::Rect(0, 0, yc.inputW, yc.inputH);
    }
    resizedSz = resized.size();
    blob = cv::dnn::blobFromImage(resized, yc.scale, cv::Size(yc.inputW, yc.inputH),
        yc.mean, yc.swapRB, false);
}

static void MakeBlobResizeCls(const cv::Mat& bgr, const ClsConfig& cc, cv::Mat& blob)
{
    cv::Mat resized; cv::resize(bgr, resized, cv::Size(cc.inputW, cc.inputH));
    blob = cv::dnn::blobFromImage(resized, cc.scale, cv::Size(cc.inputW, cc.inputH),
        cc.mean, cc.swapRB, false);
}

// YOLO�o�͂̔ėp�f�R�[�h�iv5/v8�z��j: out��(1,N,85) or (1,85,N) or (1,84,N) ��
static void DecodeYoloGeneric(const cv::Mat& out, const YoloConfig& yc,
    const cv::Size& orig, const cv::Rect& padRect,
    std::vector<int>& keepIdx,
    std::vector<cv::Rect>& boxesPx,
    std::vector<float>& scores,
    std::vector<int>& classIds)
{
    // out��3�����e���\���z��
    CV_Assert(out.dims == 3);
    // OpenCV��Mat��[C, H, W]���ۂ����Ԃ��Ƃ�����̂�size[]�Ŕ��f
    int a = out.size[1]; // ���Ԏ���
    int b = out.size[2];

    // �ǂ���̕��т�����
    bool channels_first = (a == 84 || a == 85 || a == 116); // 84=v8 coco80, 85=v5 coco80(obj��)
    int C, N; // C=�������i4+[1?]+num_classes�j, N=��␔
    if (channels_first) { C = a; N = b; }
    else { C = b; N = a; }

    const float* p = (const float*)out.data;
    // �t�ˉe�i���́����摜�j�Bletterbox���̃X�P�[�����O�ƃI�t�Z�b�g��߂�
    float gain = std::min(yc.inputW / (float)orig.width, yc.inputH / (float)orig.height);
    float padX = padRect.x;
    float padY = padRect.y;

    // ��U���ׂďW�߂�iNMS�O�j
    std::vector<cv::Rect>  tmpBoxes;
    std::vector<float>     tmpScores;
    std::vector<int>       tmpCls;
    tmpBoxes.reserve(N);
    tmpScores.reserve(N);
    tmpCls.reserve(N);

    for (int i = 0; i < N; ++i) {
        const float cx = channels_first ? p[0 * N + i] : p[i * C + 0];
        const float cy = channels_first ? p[1 * N + i] : p[i * C + 1];
        const float w = channels_first ? p[2 * N + i] : p[i * C + 2];
        const float h = channels_first ? p[3 * N + i] : p[i * C + 3];

        // v5�� obj_conf ������Av8�͖����i�G�N�X�|�[�^�ɂ�鍷�ق���j
        float obj = 1.0f;
        int clsStart = 4;
        if (C >= 85) { obj = channels_first ? p[4 * N + i] : p[i * C + 4]; clsStart = 5; }

        int bestId = -1; float bestScore = -1.0f;
        for (int c = clsStart; c < C; ++c) {
            float s = channels_first ? p[c * N + i] : p[i * C + c];
            float conf = (float)(s * obj);
            if (conf > bestScore) { bestScore = conf; bestId = c - clsStart; }
        }
        if (bestScore < yc.confThreshold) continue;

        // ���͋�Ԃ� cx,cy,w,h (YOLO�͒��S�) �� ���^�{���O���Č��摜��
        float x = cx - w * 0.5f;
        float y = cy - h * 0.5f;
        // ���^�[�{�b�N�X�␳�i���́��p�f�B���O���������摜�X�P�[���ցj
        float x0 = (x - padX) / gain;
        float y0 = (y - padY) / gain;
        float x1 = (x + w - padX) / gain;
        float y1 = (y + h - padY) / gain;

        // �N���b�v
        x0 = std::max(0.0f, std::min(x0, (float)orig.width - 1));
        y0 = std::max(0.0f, std::min(y0, (float)orig.height - 1));
        x1 = std::max(0.0f, std::min(x1, (float)orig.width - 1));
        y1 = std::max(0.0f, std::min(y1, (float)orig.height - 1));

        cv::Rect box((int)std::round(x0), (int)std::round(y0),
            (int)std::round(x1 - x0), (int)std::round(y1 - y0));
        if (box.width <= 0 || box.height <= 0) continue;

        tmpBoxes.push_back(box);
        tmpScores.push_back(bestScore);
        tmpCls.push_back(bestId);
    }

    // NMS
    std::vector<int> idx;
    cv::dnn::NMSBoxes(tmpBoxes, tmpScores, yc.confThreshold, yc.nmsThreshold, idx);

    keepIdx = idx;
    boxesPx.reserve(idx.size());
    scores.reserve(idx.size());
    classIds.reserve(idx.size());
    for (int k : idx) {
        boxesPx.push_back(tmpBoxes[k]);
        scores.push_back(tmpScores[k]);
        classIds.push_back(tmpCls[k]);
    }
}

// ���ށitop-k�j
static void DecodeClassification(const cv::Mat& out, int topk,
    std::vector<ClsScore>& scoresOut)
{
    cv::Mat logits = out;
    if (out.dims == 4 && out.size[2] == 1 && out.size[3] == 1) {
        logits = out.reshape(1, 1); // (1, C)
    }
    if (logits.rows == 1) {
        cv::Mat prob;
        // Replace the line causing the error:
        //cv::softmax(logits, prob);
        cv::Mat expLogits;
        cv::exp(logits, expLogits); // Compute the exponential of logits
        cv::reduce(expLogits, prob, 0, cv::REDUCE_SUM); // Compute the sum of exponentials
        prob = expLogits / prob; // Normalize to get softmax probabilities

        prob = prob.reshape(1, prob.cols);
        std::vector<int> idx(prob.cols);
        //std::iota(idx.begin(), idx.end(), 0);
        // Ensure this line is present at the top of the file to use std::iota
        std::partial_sort(idx.begin(), idx.begin() + std::min(topk, (int)idx.size()), idx.end(),
            [&](int a, int b) { return prob.at<float>(0, a) > prob.at<float>(0, b); });

        scoresOut.clear();
        for (int i = 0; i < std::min(topk, (int)idx.size()); ++i) {
            ClsScore s; s.classId = idx[i]; s.prob = prob.at<float>(0, idx[i]);
            scoresOut.push_back(s);
        }
    }
}

static inline cv::Rect2f PxToNormXYWH(const cv::Rect& r, const cv::Size& sz) {
    return cv::Rect2f(r.x / (float)sz.width, r.y / (float)sz.height,
        r.width / (float)sz.width, r.height / (float)sz.height);
}

DnnResult DnnInfer(const cv::Mat& bgr,
    const std::wstring& onnxPath,
    const DnnParams& params)
{
    DnnResult res;
    if (bgr.empty()) return res;

    cv::dnn::Net net = LoadOrGetNet(onnxPath, params.opt);

    // �O����
    int64 t0 = cv::getTickCount();
    cv::Mat blob;
    cv::Size resizedSz;
    cv::Rect padRect;
    cv::Mat out;

    DnnModelKind kind = params.kind;

    if (kind == DNN_KIND_YOLO || kind == DNN_KIND_AUTO) {
        MakeBlobResizeLetterbox(bgr, params.yolo, blob, resizedSz, padRect);
        net.setInput(blob);
        int64 t1 = cv::getTickCount();
        out = net.forward();
        res.inferMs = (cv::getTickCount() - t1) * 1000.0 / cv::getTickFrequency();

        // YOLO�炵���o�͂��m�F�i3�����A��������>=80+4�j
        bool looksYolo = (out.dims == 3 &&
            (out.size[1] >= 80 + 4 || out.size[2] >= 80 + 4));
        if (!looksYolo && kind == DNN_KIND_AUTO) {
            // ���ނƂ��Ĉ�������
            MakeBlobResizeCls(bgr, params.cls, blob);
            net.setInput(blob);
            int64 t2 = cv::getTickCount();
            out = net.forward();
            res.inferMs = (cv::getTickCount() - t2) * 1000.0 / cv::getTickFrequency();
            kind = DNN_KIND_CLS;
        }
        else {
            kind = DNN_KIND_YOLO;
        }
    }
    else { // ����
        MakeBlobResizeCls(bgr, params.cls, blob);
        net.setInput(blob);
        int64 t1 = cv::getTickCount();
        out = net.forward();
        res.inferMs = (cv::getTickCount() - t1) * 1000.0 / cv::getTickFrequency();
    }

    res.preprocessMs = (cv::getTickCount() - t0) * 1000.0 / cv::getTickFrequency();

    // �㏈��
    int64 t3 = cv::getTickCount();
    if (kind == DNN_KIND_YOLO) {
        std::vector<int> keep, classIds; std::vector<cv::Rect> boxesPx; std::vector<float> scores;
        DecodeYoloGeneric(out, params.yolo, bgr.size(), padRect, keep, boxesPx, scores, classIds);

        res.dets.clear();
        res.dets.reserve(boxesPx.size());
        for (size_t i = 0; i < boxesPx.size(); ++i) {
            DetBox d; d.classId = classIds[i]; d.conf = scores[i];
            if (params.yolo.boxFormat == BOX_XYWH_NORM) {
                d.box = PxToNormXYWH(boxesPx[i], bgr.size());
            }
            else if (params.yolo.boxFormat == BOX_XYWH_PIXEL) {
                d.box = cv::Rect2f((float)boxesPx[i].x, (float)boxesPx[i].y,
                    (float)boxesPx[i].width, (float)boxesPx[i].height);
            }
            else { // XYXY pixel
                float x0 = (float)boxesPx[i].x;
                float y0 = (float)boxesPx[i].y;
                float x1 = x0 + (float)boxesPx[i].width;
                float y1 = y0 + (float)boxesPx[i].height;
                d.box = cv::Rect2f(x0, y0, x1, y1);
            }
            res.dets.push_back(d);
        }
    }
    else { // ����
        DecodeClassification(out, params.cls.topk, res.cls);
    }
    res.postMs = (cv::getTickCount() - t3) * 1000.0 / cv::getTickFrequency();

    return res;
}
*/