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
    Gdiplus::Color  color; // 色
	int 		 penWidth; // ペンの幅
	Gdiplus::DashStyle dashStyle; // ダッシュスタイル

	// コンストラクタ
    LabelObj();
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
    std::vector<std::wstring>   imgPaths; // 画像ファイルのパスを格納する配列
    size_t                      imgIdx; // 現在の画像インデックス
	std::wstring                imgFolderPath; // 現在の画像フォルダ

    // 矩形の配列
	std::vector<LabelObj>     objs; // 矩形の配列
     
    // マウスドラッグ中
    LabelObj                  obj_tmp; // 矩形の座標
    bool    makeBox ; // マウスドラッグ中かどうか	
    size_t  rectIndex; // 現在の矩形インデックス

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
