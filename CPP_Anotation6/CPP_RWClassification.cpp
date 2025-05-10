#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFunctions.h"
#include "CPP_Anotation6.h"

// 文字列前後の空白削除
static std::wstring Trim(const std::wstring& s) {
    const auto start = s.find_first_not_of(L" \t\r\n");
    if (start == std::wstring::npos) return L"";
    const auto end = s.find_last_not_of(L" \t\r\n");
    return s.substr(start, end - start + 1);
}

// DashStyle ⇔ 文字列 変換
static Gdiplus::DashStyle StringToDashStyle(const std::wstring& str) {
    if (str == L"DashStyleSolid")       return Gdiplus::DashStyleSolid;
    if (str == L"DashStyleDash")        return Gdiplus::DashStyleDash;
    if (str == L"DashStyleDot")         return Gdiplus::DashStyleDot;
    if (str == L"DashStyleDashDot")     return Gdiplus::DashStyleDashDot;
    if (str == L"DashStyleDashDotDot")  return Gdiplus::DashStyleDashDotDot;
    if (str == L"DashStyleCustom")      return Gdiplus::DashStyleCustom;
    // デフォルト
    return Gdiplus::DashStyleSolid;
}

static std::wstring DashStyleToString(Gdiplus::DashStyle ds) {
    switch (ds) {
    case Gdiplus::DashStyleDash:       return L"DashStyleDash";
    case Gdiplus::DashStyleDot:        return L"DashStyleDot";
    case Gdiplus::DashStyleDashDot:    return L"DashStyleDashDot";
    case Gdiplus::DashStyleDashDotDot: return L"DashStyleDashDotDot";
    case Gdiplus::DashStyleCustom:     return L"DashStyleCustom";
    case Gdiplus::DashStyleSolid:
    default:                           return L"DashStyleSolid";
    }
}
////////////////////////////////////////////////////////////////////////////////
/// @brief クラス分類設定の読み書き
/// @param _filepath    入出力ファイルパス（ワイド文字列）
/// @param _clsNames    クラス名（wstring）
/// @param _clsColors   クラスカラー（Gdiplus::Color）
/// @param _dashStyles  線種（Gdiplus::DashStyle）
/// @param _clsPenWidths  ペン幅（int）
/// @param _rw          0=読み込み、1=書き込み
/// @return 0=成功、負値=エラー
///
int LoadClassification(
    const std::wstring& _filepath,
    std::vector<std::wstring>& _clsNames,
    std::vector<Gdiplus::Color>& _clsColors,
    std::vector<Gdiplus::DashStyle>& _dashStyles,
    std::vector<int>& _clsPenWidths,
    int  _rw
) {
    if (_rw == 1) {
        // 書き込み
        std::wofstream ofs(_filepath);
        if (ofs)
        {
            const size_t n = _clsNames.size();
            for (size_t i = 0; i < n; ++i) {
                const auto& name = _clsNames[i];
                const auto& col = _clsColors[i];
                const auto  ds = _dashStyles[i];
                const auto  w = _clsPenWidths[i];
                ofs
                    << i << L", "
                    << name << L", "
                    << col.GetR() << L", "
                    << col.GetG() << L", "
                    << col.GetB() << L", "
                    << DashStyleToString(ds) << L", "
                    << w
                    << L"\n";
            }
        }
        else
        {
			return -1; // ファイルオープン失敗
        }
    }
    else if (_rw == 0) 
    {
        // 読み込み
        std::wifstream ifs(_filepath);
        if (!ifs) return -2; // ファイルオープン失敗
        _clsNames.clear();
        _clsColors.clear();
        _dashStyles.clear();
        _clsPenWidths.clear();
		
        // 1行ずつ読み込み
        std::wstring line;
        while (std::getline(ifs, line)) 
        {
            if (!line.empty()) //データがあれば
            {
                std::wstringstream ss(line);
                std::wstring token;
                std::vector<std::wstring> tokens;
                // カンマで区切る。パースとも言う。
                while (std::getline(ss, token, L','))
                {
                    tokens.push_back(Trim(token));
                }
                if (tokens.size() == 7)
                {
                    // インデックスは tokens[0]（使わない）
                    // tokens[1]: 名称
                    _clsNames.push_back(tokens[1]);
                    // tokens[2-4]: R,G,B
                    int r = std::stoi(tokens[2]);
                    int g = std::stoi(tokens[3]);
                    int b = std::stoi(tokens[4]);
                    _clsColors.emplace_back(r, g, b);
                    // tokens[5]: DashStyle
                    _dashStyles.push_back(StringToDashStyle(tokens[5]));
                    // tokens[6]: ペン幅
                    _clsPenWidths.push_back(std::stoi(tokens[6]));
                }
                else // データ数が不正
                {
                    continue;
                }
            }
            else // 空行はスキップ
            {
                continue;
            }
        }
    }
    else 
    {
        return -3; // _rw が0/1以外
    }
    return 0;
}
