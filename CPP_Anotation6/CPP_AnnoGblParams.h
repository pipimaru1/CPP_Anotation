#pragma once

enum class DragMode { None, MakeBox, ReBox, dummy};
enum class EditMode { None, Left, Right, Top, Bottom, LeftTop, RightTop, LeftBottom, RightBottom };

///////////////////////////////////////////////////////
// アノテーションクラス
class LabelObj
{
public:
    //座標情報
    Gdiplus::RectF  Rct; //正規化された矩形
	Gdiplus::RectF  Rct_Scale; // スケールされた矩形 

public:
    //クラシフィケーション情報
    std::wstring    ClassName; // ラベル
    int             ClassNum; // 現在のラベルインデックス

	//描画情報
    Gdiplus::Color      color; // 色
	int 		        penWidth; // ペンの幅
	Gdiplus::DashStyle  dashStyle; // ダッシュスタイル

    bool mOver; //マウスがオーバーラップしているときの辺を表す

	// コンストラクタ
    LabelObj();
};

///////////////////////////////////////////////////////
// 画像クラス
// 1つの画像のパスと矩形の配列を持つクラス
struct ImgObject
{
    std::wstring            path;     // 画像のパス
    std::vector<LabelObj>   objs;     // 矩形の配列
    size_t                  objIdx{0};// 現在の矩形インデックス
    std::unique_ptr<Gdiplus::Image> image;  // 生ポインタは禁止！
    int mOverIdx = -1;// マウスオーバーのインデックス
	bool isEdited = false; // 編集されたかどうか

    // コンストラクタはデフォルトでOK
    ImgObject();
    // デストラクタもデフォルトでＯＫ
    ~ImgObject();

    // コピーは禁止（生ポインタ二重解放を防ぐ）
    ImgObject(const ImgObject&) = delete;
    ImgObject& operator=(const ImgObject&) = delete;

    // ムーブはデフォルトでＯＫ（unique_ptr がムーブされる）
    ImgObject(ImgObject&&) noexcept = default;
    ImgObject& operator=(ImgObject&&) noexcept = default;

};

///////////////////////////////////////////////////////
// グローバル変数の定義
// 課題ではヘッダファイルに記載だが、教材のためにここに記載します。
struct GlobalParams
{
    //ウィンドウサイズ
    RECT rect_win;
    int width;
    int height;

    //イメージファイル関連
	std::vector<ImgObject>      imgObjs; // 画像ファイルのパスと矩形の配列
    size_t                      imgIdx; // 現在の画像インデックス
	std::wstring                imgFolderPath; // 現在の画像フォルダ
    std::wstring                labelFolderPath; // 現在のラベルフォルダ

    // マウスドラッグ中
    LabelObj  tmpLabel;    // 矩形の座標
	DragMode dgMode = DragMode::None; // ドラッグモード

	//マウス移動中
	bool        isMouseMoving; // マウス移動中かどうか
    POINT       g_prevPt;
    int         Overlap; // マウスオーバーの裕度

    //矩形ドラッグ
	int activeIdx   = -1; // ドラッグ中の矩形のインデックス
	EditMode edMode = EditMode::None;// マウスオーバー中の矩形の辺を表す
    Gdiplus::PointF prevMouse;   // 前回のマウス座標

    // 対象とする画像拡張子パターン
    std::vector<std::wstring>   IMAGE_EXTENSIONS;

	// クラシフィケーションの名前
    std::vector<std::wstring>       ClsNames;
    std::vector<Gdiplus::Color>     ClsColors;
    std::vector<Gdiplus::DashStyle> ClsDashStyles;
	std::vector<int>                ClsPenWidths;

    // フォント関連はポインタに
    Gdiplus::FontFamily* fontFamily;
    Gdiplus::Font* font;

	int selectedClsIdx; // 選択されたクラシフィケーションのインデックス

    // フォント生成を遅延させるメソッド
    void InitFont();
    void DestroyFont();

    //コンストラクタ
    GlobalParams();
    ~GlobalParams();

};

extern GlobalParams GP;
