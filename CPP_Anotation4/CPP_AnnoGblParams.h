#pragma once
///////////////////////////////////////////////////////
// アノテーションクラス
struct LabelObj
{
    //座標情報
	Gdiplus::RectF  rect; // 矩形の座標

    //クラシフィケーション情報
    std::wstring    ClassName; // ラベル
    int             CalassNum; // 現在のラベルインデックス

	//描画情報
    Gdiplus::Color      color; // 色
	int 		        penWidth; // ペンの幅
	Gdiplus::DashStyle  dashStyle; // ダッシュスタイル

	// コンストラクタ
    LabelObj();
};

///////////////////////////////////////////////////////
// 画像クラス
// 1つの画像のパスと矩形の配列を持つクラス
struct ImgObject
{
    std::wstring path;          // 画像のパス
    std::vector<LabelObj> objs;   // 矩形の配列
    size_t      objIdx{0};             // 現在の矩形インデックス

    std::unique_ptr<Gdiplus::Image> image;  // 生ポインタは禁止！

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

    // マウスドラッグ中
    LabelObj  anno_tmp;    // 矩形の座標
    bool        makeBox; // マウスドラッグ中かどうか	

	//マウス移動中
	bool        isMouseMoving; // マウス移動中かどうか
    POINT       g_prevPt;

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
    //Gdiplus::SolidBrush  fontBrush;

	int selectedClsIdx; // 選択されたクラシフィケーションのインデックス

    // フォント生成を遅延させるメソッド
    void InitFont();
    void DestroyFont();

    //コンストラクタ
    GlobalParams();
    ~GlobalParams();

};

extern GlobalParams GP;
