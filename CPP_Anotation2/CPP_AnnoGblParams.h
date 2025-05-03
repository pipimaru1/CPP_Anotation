#pragma once


///////////////////////////////////////////////////////
// グローバル変数の定義
// 課題ではヘッダファイルに記載だが、教材のためにここに記載します。
struct GlobalParams
{
    //ウィンドウサイズ
    RECT rect_win;
    int width;
    int height;

    //イメージファイル
    std::vector<std::wstring> imgPaths; // 画像ファイルのパスを格納する配列
    size_t imgIndex; // 現在の画像インデックス

    // 矩形の配列
    std::vector<Gdiplus::RectF> rects; // 矩形の配列
    // マウスドラッグ中
    Gdiplus::RectF rect_tmp; // 矩形の座標
    bool isDragging = false; // マウスドラッグ中かどうか	
    size_t rectIndex = 0; // 現在の矩形インデックス

    // 対象とする画像拡張子パターン
    std::vector<std::wstring> IMAGE_EXTENSIONS;
    GlobalParams();
};

extern GlobalParams GP;
