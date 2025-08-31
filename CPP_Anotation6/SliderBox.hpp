//************* SliderBox.h
#pragma once
#include <windows.h>
#include <string>
#include <CommCtrl.h>
#include <sstream>
#include <iomanip>


//////////////////////////////////////////////////////////////////////////////////
class MkFont
{
    HFONT    hFont;

public:
    /// 指定コントロールにフォントを作成・適用し、その HFONT を返す
    /// @param hwndCtrl フォントを設定するコントロールの HWND
    /// @param pointSize フォントサイズ（ポイント）
    /// @param isBold    太字にする場合 true（通常は false）
    /// @return 作成した HFONT（不要時は DeleteObject で解放を）
    HFONT mkFont(HWND hwndCtrl, int pointSize, bool isBold)
    {
        // 画面の DPI（垂直方向）を取得
        HDC hdc = GetDC(hwndCtrl);
        int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(hwndCtrl, hdc);

        // ポイントサイズから論理ピクセル単位へ変換（負値でキャラクタ高さ指定）
        int height = -MulDiv(pointSize, dpiY, 72);

        // フォント作成（例：MS UI Gothic）
        hFont = CreateFontW(
            height,           // nHeight
            0, 0, 0,          // nWidth, nEscapement, nOrientation
            isBold ? FW_BOLD : FW_NORMAL,  // フォントウェイト
            FALSE, FALSE, FALSE,            // イタリック，アンダーライン，ストライクアウト
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_SWISS,
            L"MS UI Gothic"  // フォント名
        );

        // コントロールに適用（TRUE で再描画）
        SendMessageW(hwndCtrl, WM_SETFONT, (WPARAM)hFont, TRUE);

        return hFont;
    }

    /////////////////////////////
    MkFont()
        : hFont(nullptr) // 初期化
    {
    }
    /////////////////////////////
    MkFont(HWND hwndCtrl, int pointSize, bool isBold)
        : hFont(mkFont(hwndCtrl, pointSize, isBold))
    {
        // コンストラクタでフォントを作成・適用
    }

    /////////////////////////////
    /// デストラクタでフォントを解放
    ~MkFont() {
        if (hFont) {
            DeleteObject(hFont);
        }
    }
};
//RECT_XYWH dlgRECT = { 0, 0, 240, 40 }; // 基本の矩形サイズ（スライダーの位置とサイズを保持）
#define SLBOX_FONTSIZE 10
#define SLBOX_FONTSIZEV 8
#define SIZE_EXITBTN 15 // Xボタンのサイズ

//////////////////////////////////////////////////////////////////////////////////
//
struct RECT_XYWH
{
    int x, y, w, h;
};


//////////////////////////////////////////////////////////////////////////////////
// スライダーボックスを管理するクラス
// スライダーボックスはint long float double のいずれかの値をバインドする
// 共通部分を なるべく SliderBase として切り出し、
// SliderBox と SliderDialog がそれを継承する形にする
//////////////////////////////////////////////////////////////////////////////////
class SliderBase
{
public:
    // 各コントロールの位置とサイズを保持するための構造体
    RECT_XYWH dlgRECT; //ダイアログボックスの大きさ
    RECT_XYWH rcArea; //= { 0, 0, 240, 40 }; // 全体の大きさを保持
    RECT_XYWH rcLabel; //= { 0, 0, 80, 20 }; // ラベルの位置とサイズを保持
    RECT_XYWH rcValue; //= { 80, 0, 80, 20 }; // ラベルの位置とサイズを保持
    RECT_XYWH rcSlider; // = { 0, 20, 240, 20 }; // スライダーの位置とサイズを保持

    // フォント関係
    MkFont fnLabel; // フォント作成用ヘルパークラス
    MkFont fnValue; // フォント作成用ヘルパークラス
    MkFont fnSlider; // フォント作成用ヘルパークラス

    //ハンドル関係
    HWND       m_parent;
    HWND       m_hwndGroup;     // タイトル付き枠
    HWND       m_hwndLabelName; // 値名ラベル
    HWND       m_hwndTrack;     // トラックバー
    HWND       m_hwndLabelVal;  // 値表示ラベル

    //値関係
    std::wstring m_name;

    // トラックバーの目盛り数
    static constexpr int TICKS = 20000;

    //縦か横か
    bool        m_vertical;

public:
    SliderBase();

    void Init(
        HWND parent,
        int _x0, int _y0, int _w0, int _h0,
        bool vertical,
        const std::wstring& name_of_value);

    // トラックバーの位置を value に変換してラベルに反映
    inline int flipPos(int pos) const
    {
        if (m_vertical)
        {
            // 縦型の場合は反転
            return (TICKS - pos);
        }
        else
            return pos; // 横型の場合はそのまま
    }

};

///////////////////////////////////////////////////////////////////////////
SliderBase::SliderBase()
    : m_parent(nullptr), m_hwndGroup(nullptr), m_hwndLabelName(nullptr),
    m_hwndTrack(nullptr), m_hwndLabelVal(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////
void SliderBase::Init(
    HWND parent,
    int _x0, int _y0, int _w0, int _h0,
    bool vertical,
    const std::wstring& name_of_value)
{
    //基本的な配置
    rcArea = { 0, 0, 240, 40 };       // 全体の大きさ
    rcLabel = { 0, 0, 120, 20 };     // ラベルの位置とサイズを保持
    rcValue = { 120, 0, 60, 20 };    // ラベルの位置とサイズを保持
    rcSlider = { 0, 20, 240, 20 };  // スライダーの位置とサイズを保持

    m_parent = parent;
    m_vertical = vertical;
    m_name = name_of_value;
    // 縦横の位置とサイズを設定
    dlgRECT.x = _x0;
    dlgRECT.y = _y0;
    dlgRECT.w = _w0;
    dlgRECT.h = _h0;

    // 共通コントロールを確実に初期化
    INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_BAR_CLASSES };
    InitCommonControlsEx(&icex);

    //縮尺
    //float _xr = static_cast<float>(_w0) / static_cast<float>(dlgRECT.w);
    //float _yr = static_cast<float>(_h0) / static_cast<float>(dlgRECT.h);

    float _xr = static_cast<float>(_w0) / static_cast<float>(rcArea.w);
    float _yr = static_cast<float>(_h0) / static_cast<float>(rcArea.h);


    RECT_XYWH rc;

    // ラベルの位置とサイズを計算
    // 横
    if (!m_vertical)
    {
        ///////////////////////////////////////////////////////////////
        // 値名ラベル　左上を想定
        rc.x = rcLabel.x * _xr + _x0;
        rc.y = rcLabel.y * _yr + _y0; // +SLBOX_FONTSIZE;
        rc.w = rcLabel.w * _xr;
        rc.h = rcLabel.h * _yr;

        m_hwndLabelName = CreateWindowExW(
            0, L"STATIC", m_name.c_str(),
            WS_CHILD | WS_VISIBLE,
            rc.x, rc.y, rc.w, rc.h,
            m_parent, nullptr, GetModuleHandle(nullptr), nullptr);

        fnLabel.mkFont(m_hwndLabelName, SLBOX_FONTSIZE, true);
        ///////////////////////////////////////////////////////////////
        // 値表示ラベル
        rc.x = rcValue.x * _xr + _x0;
        rc.y = rcValue.y * _yr + _y0;// +SLBOX_FONTSIZE;;
        rc.w = rcValue.w * _xr;
        rc.h = rcValue.h * _yr;
        m_hwndLabelVal = CreateWindowExW(
            0, L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            rc.x, rc.y, rc.w, rc.h,
            //rc.left + w - 88, rc.top, 80, 20,
            m_parent, nullptr, GetModuleHandle(nullptr), nullptr);
        fnValue.mkFont(m_hwndLabelVal, SLBOX_FONTSIZE, true);
        ///////////////////////////////////////////////////////////////
        // トラックバー本体
        rc.x = rcSlider.x * _xr + _x0;
        rc.y = rcSlider.y * _yr + _y0;
        rc.w = rcSlider.w * _xr;
        rc.h = rcSlider.h * _yr;

        DWORD style = WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_HORZ;

        m_hwndTrack = CreateWindowExW(
            0, TRACKBAR_CLASS, nullptr,
            style,
            rc.x, rc.y, rc.w, rc.h,
            //rc.left + 80, rc.top, rc.right - rc.left - 160, 20,
            m_parent, nullptr, GetModuleHandle(nullptr), nullptr);

        fnSlider.mkFont(m_hwndLabelVal, SLBOX_FONTSIZE, true);
        ///////////////////////////////////////////////////////////////
        // 目盛り設定
        SendMessage(m_hwndTrack, TBM_SETRANGE, TRUE, MAKELPARAM(0, TICKS));
        SendMessage(m_hwndTrack, TBM_SETTICFREQ, TICKS / 10, 0);
    }
    ///////////////////////////////////////////////////////////////
    // 縦型の場合は rcSlider の位置とサイズを調整
    else
    {
        rc.x = rcSlider.x * _xr + _x0;
        rc.y = rcSlider.y * _yr + _y0;
        rc.w = _w0;
        rc.h = _h0;

        ///////////////////////////////////////////////////////////////
        //if (titlebar) {
        //    m_hwndGroup = CreateWindowExW(
        //        0, L"BUTTON", m_name.c_str(),
        //        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        //        rc.x, rc.y, rc.w, rc.h,
        //        m_parent, nullptr, GetModuleHandle(nullptr), nullptr);
        //    // 内側にマージンを取る
        //    //InflateRect(&rc, -8, -20);
        //}

        ///////////////////////////////////////////////////////////////
        // 値名ラベル
        rc.x = 0;
        rc.y = SIZE_EXITBTN;
        rc.w = _w0;
        rc.h = rcLabel.h;

        m_hwndLabelName = CreateWindowExW(
            0, L"STATIC", m_name.c_str(),
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            rc.x, rc.y, rc.w, rc.h,
            m_parent, nullptr, GetModuleHandle(nullptr), nullptr);

        fnLabel.mkFont(m_hwndLabelName, SLBOX_FONTSIZEV, true);
        ///////////////////////////////////////////////////////////////
        // 値表示ラベル
        rc.x = 0;
        rc.y = rcLabel.h + SIZE_EXITBTN;
        rc.w = _w0;
        rc.h = rcValue.h;
        m_hwndLabelVal = CreateWindowExW(
            0, L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            rc.x, rc.y, rc.w, rc.h,
            //rc.left + w - 88, rc.top, 80, 20,
            m_parent, nullptr, GetModuleHandle(nullptr), nullptr);
        fnValue.mkFont(m_hwndLabelVal, SLBOX_FONTSIZE, true);
        ///////////////////////////////////////////////////////////////
        // トラックバー本体
        rc.x = _w0 / 2 - 20;
        rc.y = rcValue.h + rcLabel.h + SIZE_EXITBTN;
        rc.w = _w0;
        rc.h = _h0 - rc.y;

        DWORD style = WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT | TBS_DOWNISLEFT;

        m_hwndTrack = CreateWindowExW(
            0, TRACKBAR_CLASS, nullptr,
            style,
            rc.x, rc.y, rc.w, rc.h,
            //rc.left + 80, rc.top, rc.right - rc.left - 160, 20,
            m_parent, nullptr, GetModuleHandle(nullptr), nullptr);

        fnSlider.mkFont(m_hwndLabelVal, SLBOX_FONTSIZE, true);
        ///////////////////////////////////////////////////////////////
        // 目盛り設定
        SendMessage(m_hwndTrack, TBM_SETRANGE, TRUE, MAKELPARAM(0, TICKS));
        SendMessage(m_hwndTrack, TBM_SETTICFREQ, TICKS / 10, 0);
    }
    // 初期位置を設定
    //SetValue(m_value);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//スライダークラス 汎用型
////////////////////////////////////////////////////////////////////////////////////////////////////
//テンプレートで拡張
//スライダーボックスはint long float double のいずれかの値をバインドする
template<typename T>
class SliderBoxT {
private:
    SliderBase Base; // スライダーボックスの共通部分を保持

    //保持する値はこの三つだけ
    T& m_value;
    T m_min;
    T m_max;

public:
    // フル初期化コンストラクタ
    // parent       : スライダーを配置する親ウィンドウ
    // x, y, w, h   : 配置・サイズ
    // value        : バインドする float 変数の参照
    // min, max     : スライダーの最小／最大値
    // vertical     : 縦型なら true、横型なら false
    // titlebar     : タイトルバー（グループボックス）を表示するなら true
    // name_of_value: 値名（左側に表示）
    SliderBoxT(
        HWND parent,
        int  x, int y, int w, int h,
        T& value,
        T    min, T max,
        bool vertical,
        const std::wstring& name_of_value
    );

    //最低限の値のみ初期化するコンストラクタ
    SliderBoxT(
        T& value,
        T min, T max
    );

    ~SliderBoxT();

    void Init(HWND parent,
        int x, int y, int w, int h,
        bool vertical,
        const std::wstring& name_of_value);


    // WM_HSCROLL／WM_VSCROLL を受け取ったら呼ぶ
    T HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    // スライダー値を取得
    T GetValue() const { return m_value; }
    // スライダー値を外部から設定（位置・ラベルも更新される）
    void SetValue(T _value);

private:
    // トラックバーの位置を value に変換
    T UpdateValueFromPos(int pos);
    // value をラベル文字列に反映
    T UpdateLabel();
};
////////////////////////////////////////
template<typename T>
SliderBoxT<T>::SliderBoxT(HWND parent,
    int _x0, int _y0, int _w0, int _h0,
    T& value,
    T min, T max,
    bool vertical,
    const std::wstring& name_of_value)
    : m_value(value)
    , m_min(min)
    , m_max(max)
{
    Init(parent,
        _x0, _y0, _w0, _h0,
        vertical,
        name_of_value);
}
// 簡易コンストラクタ
template<typename T>
SliderBoxT<T>::SliderBoxT(
    T& value,
    T  min, T max
)
    : Base()
    , m_value(value)
    , m_min(min)
    , m_max(max)
{
    // 後で Init() を呼ぶ
}
//////////////////////////////////////////////////////////////////////////////////
template<typename T>
SliderBoxT<T>::~SliderBoxT()
{
    // ウィンドウは親が破棄すると自動破棄される想定なので特に削除不要
}
//////////////////////////////////////////////////////////////////////////////////
template<typename T>
void SliderBoxT<T>::Init(HWND parent, int _x0, int _y0, int _w0, int _h0, bool vertical, const std::wstring& name_of_value)
{
    // 共通部分を初期化
    Base.Init(parent, _x0, _y0, _w0, _h0, vertical, name_of_value);

    // 初期位置を設定
    SetValue(m_value);
}

//////////////////////////////////////////////////////////////////////////////////
template<typename T>
void SliderBoxT<T>::SetValue(T _value)
{
    if (_value < m_min)
        _value = m_min;
    if (_value > m_max)
        _value = m_max;
    m_value = _value;
    // 値→ポジション
    //int pos = static_cast<int>((_value - m_min) / (m_max - m_min) * Base.TICKS);

    // 浮動小数点で比率を計算
    double ratio = (static_cast<float>(m_value) - static_cast<float>(m_min))
        / (static_cast<float>(m_max) - static_cast<float>(m_min));

    // スライダー目盛にマッピング
    int pos = static_cast<int>(ratio * Base.TICKS);

    pos = Base.flipPos(pos);             //縦の時は反転する
    SendMessage(Base.m_hwndTrack, TBM_SETPOS, TRUE, pos);
    UpdateLabel();
}

//////////////////////////////////////////////////////////////////////////////////
template<typename T>
T SliderBoxT<T>::UpdateValueFromPos(int pos)
{
    pos = Base.flipPos(pos);                       // ★ここで反転
    double ratio = (static_cast<float>(pos) / static_cast<float>(Base.TICKS));
    m_value = m_min + ratio * (m_max - m_min);

    return m_value;
}

//////////////////////////////////////////////////////////////////////////////////
template<typename T>
T SliderBoxT<T>::UpdateLabel()
{
    std::wstringstream ss;
    ss.precision(2);
    ss << std::fixed << m_value;
    SetWindowTextW(Base.m_hwndLabelVal, ss.str().c_str());

    return m_value;
}

//////////////////////////////////////////////////////////////////////////////////
// WM_HSCROLL/WM_VSCROLL で呼んでください
template<typename T>
T SliderBoxT<T>::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if ((msg == WM_HSCROLL || msg == WM_VSCROLL) &&
        reinterpret_cast<HWND>(lParam) == Base.m_hwndTrack)
    {
        int pos = static_cast<int>(SendMessage(Base.m_hwndTrack, TBM_GETPOS, 0, 0));
        UpdateValueFromPos(pos);
        return UpdateLabel();
    }
    else
    {
        // 他のメッセージは無視
        return m_value;  // 変更なし
    }
}

//////////////////////////////////////////////////////////////////////////////////
// 汎用スライダーボックスクラス
template<typename T>
class SliderDialogT {
public:
    // hInst       : アプリケーションインスタンス
    // hParent     : モーダルダイアログの親ウィンドウ
    // value       : バインドする float 変数の参照
    // min, max    : スライダーの最小／最大値
    // vertical    : 縦型か横型か
    // titlebar    : グループボックス（タイトルバー）表示フラグ
    // name        : 値名（ラベル表示文字列）
    SliderDialogT(HINSTANCE hInst, HWND hParent,
        T& value,
        T min, T max,
        bool vertical,
        const std::wstring& name,
        int _x0,
        int _y0,
        int _width,
        int _height
    );
    ~SliderDialogT();

    void Destroy();

    // モーダル表示
    INT_PTR ShowModal();
    HWND    ShowModeless(HWND parent);     // ← 追加：非ブロッキング表示

private:
    //SliderBox*       m_pSlider;
    SliderBoxT<T>* m_pSlider;

    HINSTANCE      m_hInst;
    HWND           m_hParent;
    HWND           m_hDlg;
    // ここでドラッグ開始位置とウィンドウ位置を保持
    POINT           m_dragStart;
    RECT            m_winStart;

    bool           m_vertical;
    std::wstring   m_name;

    int m_x0;
    int m_y0;
    int m_width;
    int m_height;

    // ダイアログプロシージャ
    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    // インスタンス毎のメッセージハンドラ
    INT_PTR HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);
};



template<typename T>SliderDialogT<T>::SliderDialogT(HINSTANCE hInst, HWND hParent,
    T& value,
    T min, T max,
    bool vertical,
    const std::wstring& name,
    int initX,      // ← 追加
    int initY,       // ← 追加
    int _width,
    int _height
)
    : m_hInst(hInst)
    , m_hParent(hParent)
    , m_hDlg(nullptr)
    , m_pSlider(nullptr)
    //, m_min(min)
    //, m_max(max)
    , m_vertical(vertical)
    , m_name(name)
    , m_x0(initX)   // ← ここで保存
    , m_y0(initY)   // ← ここで保存
    , m_width(_width)
    , m_height(_height)
{
    m_pSlider = new SliderBoxT<T>(value, min, max);
}

////////////////////////////////////////////////
template<typename T>SliderDialogT<T>::~SliderDialogT() {
    delete m_pSlider;
}

////////////////////////////////////////////////
template<typename T>
INT_PTR SliderDialogT<T>::ShowModal() {
    // lParam に this を渡して WM_INITDIALOG で受け取る
    return DialogBoxParamW(
        m_hInst,
        MAKEINTRESOURCEW(IDD_SLIDER_DIALOG),
        //0,
        m_hParent,
        &SliderDialogT::DlgProc,
        reinterpret_cast<LPARAM>(this)
    );
}

////////////////////////////////////////////////
template<typename T>
HWND SliderDialogT<T>::ShowModeless(HWND parent) {
    // CreateDialogParamW → 即リターン
    m_hDlg = CreateDialogParamW(
        GetModuleHandle(NULL),
        MAKEINTRESOURCEW(IDD_SLIDER_DIALOG),
        //0,
        parent,
        SliderDialogT::DlgProc,
        reinterpret_cast<LPARAM>(this)
    );
    if (m_hDlg) {
        ShowWindow(m_hDlg, SW_SHOW);
    }
    //else
    //{
    //    DWORD err = GetLastError();                 // 失敗要因を取得
    //    wchar_t msg[256]{};
    //    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
    //        FORMAT_MESSAGE_IGNORE_INSERTS,
    //        nullptr, err, 0, msg, 256, nullptr);
    //    MessageBoxW(parent, msg, L"CreateDialogParamW 失敗", MB_OK);
    //    return nullptr;
    //}

    return m_hDlg;
}

////////////////////////////////////////////////
template<typename T>
void SliderDialogT<T>::Destroy()
{
    if (m_hDlg) {
        DestroyWindow(m_hDlg);
        m_hDlg = nullptr;
    }
}

////////////////////////////////////////////////
template<typename T>
INT_PTR CALLBACK SliderDialogT<T>::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    SliderDialogT* pThis = nullptr;

    if (msg == WM_INITDIALOG) {
        // lParam に渡された this ポインタを保存
        pThis = reinterpret_cast<SliderDialogT*>(lParam);
        SetWindowLongPtrW(hDlg, GWLP_USERDATA, lParam);

        // 初期化後にインスタンスメンバで扱う
        pThis->m_hDlg = hDlg;

        // タイトルバーを消す
        // 標準スタイルからキャプション／枠線を外す
        LONG_PTR style = GetWindowLongPtrW(hDlg, GWL_STYLE);
        //style &= ~(WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | WS_THICKFRAME);
        style &= ~(WS_CAPTION);// | WS_POPUP | WS_VISIBLE;
        SetWindowLongPtrW(hDlg, GWL_STYLE, style);
        // スタイル変更を反映
        SetWindowPos(hDlg, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    else {
        pThis = reinterpret_cast<SliderDialogT*>(GetWindowLongPtrW(hDlg, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->HandleMessage(msg, wParam, lParam);
    }
    return FALSE;
}

//float _DUMMY_FLOAT = 0.0f; // ダミー変数（SliderBox のコンストラクタで必要）
//float _DUMMY_MIN = 0.0f; // ダミー最小値
//float _DUMMY_MAX = 100.0f; // ダミー最大値
////////////////////////////////////////////////
template<typename T>
INT_PTR SliderDialogT<T>::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG:
    {
        // ダイアログのクライアント領域サイズを取得
        RECT rcClient;
        GetClientRect(m_hDlg, &rcClient);

        // SliderBox を生成（マージン 10px）
        int x = 0;
        int y = 0;
        int w = m_width;
        int h = m_height;  // 適宜変更

        if (m_pSlider != nullptr) {
            m_pSlider->Init(
                m_hDlg, x, y, w, h,
                m_vertical,
                m_name
            );
        }
        //else {
        //    //このブロックが呼ばれてはいけない
        //    m_pSlider = new SliderBoxT<T>(
        //        m_hDlg, x, y, w, h,
        //        _DUMMY_FLOAT,
        //        _DUMMY_MIN, _DUMMY_MAX,
        //        m_vertical,
        //        m_name
        //    );
        //}

        if (!m_vertical)
        {
            //Xボタン
            CreateWindowExW(
                0, L"BUTTON", L"X",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
                m_width - SIZE_EXITBTN - 10, 0,
                SIZE_EXITBTN,
                SIZE_EXITBTN,
                m_hDlg, (HMENU)IDCANCEL, m_hInst, nullptr
            );
        }
        else
        {
            //Xボタン
            CreateWindowExW(
                0, L"BUTTON", L"X",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
                0, 0,
                SIZE_EXITBTN,
                SIZE_EXITBTN,
                m_hDlg, (HMENU)IDCANCEL, m_hInst, nullptr
            );
        }

        //GetClientRect(m_hDlg, &rcClient);

        if (m_x0 != CW_USEDEFAULT && m_y0 != CW_USEDEFAULT) {
            // 1) 親ウィンドウのクライアント領域を取得
            RECT rcParent;
            GetClientRect(m_hParent, &rcParent);

            // 2) (0,0) → スクリーン座標に変換
            POINT ptClient = { rcParent.left, rcParent.top };
            ClientToScreen(m_hParent, &ptClient);

            // 3) m_initX, m_initY を加算して最終表示位置を計算
            int posX = ptClient.x + m_x0;
            int posY = ptClient.y + m_y0;

            // 4) サイズはそのまま、Zオーダーも変えずに移動
            SetWindowPos(m_hDlg, HWND_TOP,
                posX, posY,
                m_width, m_height,
                SWP_NOZORDER);
        }
    }
    return TRUE;

    case WM_NCHITTEST:
        break;

    case WM_HSCROLL:
    case WM_VSCROLL:
        if (m_pSlider) {
            m_pSlider->HandleMessage(msg, wParam, lParam);
        }
        return TRUE;

        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
            EndDialog(m_hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;

    case WM_LBUTTONDOWN:
        SetCapture(m_hDlg);
        GetCursorPos(&m_dragStart);
        GetWindowRect(m_hDlg, &m_winStart);
        return TRUE;
        break;

    case WM_MOUSEMOVE:
        if (GetCapture() == m_hDlg && (wParam & MK_LBUTTON)) {
            POINT pt; GetCursorPos(&pt);
            int dx = pt.x - m_dragStart.x;
            int dy = pt.y - m_dragStart.y;
            SetWindowPos(m_hDlg, nullptr,
                m_winStart.left + dx,
                m_winStart.top + dy,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        break;

    case WM_LBUTTONUP:
        if (GetCapture() == m_hDlg) ReleaseCapture();
        break;


    }
    return FALSE;
}
