// pch.h: プリコンパイル済みヘッダー ファイルです。
// 次のファイルは、その後のビルドのビルド パフォーマンスを向上させるため 1 回だけコンパイルされます。
// コード補完や多くのコード参照機能などの IntelliSense パフォーマンスにも影響します。
// ただし、ここに一覧表示されているファイルは、ビルド間でいずれかが更新されると、すべてが再コンパイルされます。
// 頻繁に更新するファイルをここに追加しないでください。追加すると、パフォーマンス上の利点がなくなります。

#ifndef PCH_H
#define PCH_H

// プリコンパイルするヘッダーをここに追加します
// 先に記述
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <shlwapi.h>  // PathMatchSpecW 用
#include <shlobj.h> // Add this include to resolve BROWSEINFO and related functions
#include <iomanip> // Add this include for std::setprecision and std::fixed
#include <sstream> // Ensure this is included for std::wostringstream
#include <fstream> // std::wofstream を使用するために必要
#include <locale>
#include <codecvt> // Add this header for codecvt_utf8
#include <shobjidl.h>
#include <pathcch.h>      // Windows 8 SDK 以降
#include <commctrl.h>
#include <set>

#include <omp.h>    // ← 追加

//残すが不要
#include "framework.h"

#endif //PCH_H
