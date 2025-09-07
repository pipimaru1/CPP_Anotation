#include "pch.h"

#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <locale>
#include <sstream>
#include <filesystem>

#include "CPP_Menu.h"

/*
// ---------- 文字コード変換（<codecvt> 非依存） ----------
static std::wstring BytesToWString(const std::string& bytes, UINT cp, bool strictUtf8 = false)
{
	if (bytes.empty()) return L"";
	DWORD flags = 0;
	if (cp == CP_UTF8 && strictUtf8) flags = MB_ERR_INVALID_CHARS;

	int wlen = MultiByteToWideChar(cp, flags, bytes.data(), (int)bytes.size(), nullptr, 0);
	if (wlen <= 0) return L"";
	std::wstring w(wlen, 0);
	MultiByteToWideChar(cp, flags, bytes.data(), (int)bytes.size(), w.data(), wlen);
	return w;
}
*?

/////////////////////////////////////////////////////////////////////
// メニューをファイルから読み込む
/*
inline int loadmenu_onnx_old(const std::wstring& filename, std::vector<MenuItemOnnx>& menu)
{
	std::wifstream _ifs(filename);

	if (_ifs)
	{
		//文字コードの調整
		//_ifs.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>)); // ★UTF-8
		_ifs.imbue(std::locale(".932")); // ★CP932（Shift-JIS）
		//_ifs.imbue(std::locale("ja_JP.SJIS"));

		std::wstring line;
		while (std::getline(_ifs, line))
		{
			if (line.empty())
				continue; // 空行は無視
			if (line[0] == '#')
				continue; // コメント行は無視
			//一行分
			MenuItemOnnx item;
			size_t pos = 0;
			size_t comma_pos = 0;
			int field_index = 0;
			while ((comma_pos = line.find(',', pos)) != std::string::npos)
			{
				std::wstring field = line.substr(pos, comma_pos - pos);
				field.erase(0, field.find_first_not_of(L" \t")); // 前の空白を削除
				field.erase(field.find_last_not_of(L" \t") + 1); // 後の空白を削除
				switch (field_index)
				{
					case 0:
						item.Id = field;
						break;
					case 1:
						item.MenuStr = field;
						break;
					case 2:
						item.OnnxPath = field;
						break;
					case 3:
						item.ClsNamePath = field;
						break;
					case 4:
						item.YoloType = field;
						break;
					case 5:
						item.Resolution = std::stoi(field);
						break;
					case 6:
						item.ClassCount = std::stoi(field);
						break;
					default:
						break; // 不要なフィールドは無視
				}
				pos = comma_pos + 1;
				field_index++;
			}
			// 最後のフィールドを処理
			if (pos < line.size() && field_index == 5)
			{
				std::wstring field = line.substr(pos);
				field.erase(0, field.find_first_not_of(L" \t")); // 前の空白を削除
				field.erase(field.find_last_not_of(L" \t") + 1); // 後の空白を削除
				item.ClassCount = std::stoi(field);
			}
			if (field_index >= 5) // 必要なフィールドが揃っている場合のみ追加
			{
				menu.push_back(item);
			}
		}
		return 0; // 成功

	}

	else
	{
		return -1; // ファイルが開けない
	}
}
*/

// どこかヘッダの上の方（loadmenu_onnx より前）に置く

// ファイルを丸ごと読み込んで UTF-8（BOM付きも可） or CP932 を wstring に変換
static inline std::wstring ReadTextFileW(const std::wstring& path)
{
	std::ifstream fs(path, std::ios::binary);
	if (!fs) return L"";

	std::vector<char> bin((std::istreambuf_iterator<char>(fs)), {});
	if (bin.empty()) return L"";

	// UTF-8 BOM 判定
	UINT cp = 932; size_t off = 0;
	if (bin.size() >= 3 &&
		(unsigned char)bin[0] == 0xEF && (unsigned char)bin[1] == 0xBB && (unsigned char)bin[2] == 0xBF) {
		cp = CP_UTF8; off = 3;
	}

	auto in = bin.data() + off;
	auto len = (int)(bin.size() - off);

	// まずUTF-8で厳密に試す（BOMありはここに来る）
	if (cp == CP_UTF8) {
		int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in, len, nullptr, 0);
		if (wlen > 0) {
			std::wstring w(wlen, 0);
			MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in, len, &w[0], wlen);
			return w;
		}
		// 失敗したら CP932 にフォールバック
		cp = 932;
	}

	int wlen = MultiByteToWideChar(cp, 0, in, len, nullptr, 0);
	std::wstring w(wlen, 0);
	MultiByteToWideChar(cp, 0, in, len, &w[0], wlen);
	return w;
}

// テキスト（wstring）を \r\n / \n / \r すべてで行分割
static inline std::vector<std::wstring> SplitLinesW(const std::wstring& text)
{
	std::vector<std::wstring> lines;
	size_t start = 0;
	for (size_t i = 0; i < text.size(); ++i) {
		if (text[i] == L'\r' || text[i] == L'\n') {
			lines.emplace_back(text.substr(start, i - start));
			if (text[i] == L'\r' && i + 1 < text.size() && text[i + 1] == L'\n') ++i; // CRLF
			start = i + 1;
		}
	}
	if (start < text.size()) lines.emplace_back(text.substr(start));
	return lines;
}


/////////////////////////////////////////////////////////////////////
// メニューをファイルから読み込む（"..."対応版 / CSV風パース）
int loadmenu_onnx(const std::wstring& filename, std::vector<MenuItemOnnx>& menu)
{
	// （必要なら既存の Trim を使ってOK）
	auto TrimW = [](const std::wstring& s) -> std::wstring {
		const auto b = s.find_first_not_of(L" \t\r\n");
		if (b == std::wstring::npos) return L"";
		const auto e = s.find_last_not_of(L" \t\r\n");
		return s.substr(b, e - b + 1);
		};

	// CSV 1 行を "で考慮しつつ分解（, はクォート外のみ区切り / "" はクォート内のエスケープ）
	auto SplitCsvLineW = [&](const std::wstring& src) -> std::vector<std::wstring> {
		std::vector<std::wstring> out;
		std::wstring field;
		bool inQuotes = false;

		for (size_t i = 0; i < src.size(); ++i) {
			wchar_t ch = src[i];
			if (ch == L'"') {
				if (inQuotes && i + 1 < src.size() && src[i + 1] == L'"') {
					// 連続する "" はエスケープされた 1 個の " として格納
					field.push_back(L'"');
					++i;
				}
				else {
					inQuotes = !inQuotes; // 開閉トグル
				}
			}
			else if (ch == L',' && !inQuotes) {
				out.push_back(TrimW(field));
				field.clear();
			}
			else {
				field.push_back(ch);
			}
		}
		out.push_back(TrimW(field));
		return out;
		};

	// UTF-8/UTF-16 BOM を除去（SJIS 読みでも wifstream には U+FEFF が混ざることがある）
	auto StripBOM = [](std::wstring s) {
		if (!s.empty() && s[0] == 0xFEFF) s.erase(0, 1);
		return s;
		};

	std::wifstream ifs(filename);
	if (!ifs) return -1;

	ifs.imbue(std::locale(".932")); // CP932（Shift-JIS）

	std::wstring line;
	size_t lineno = 0;
	while (std::getline(ifs, line)) {
		++lineno;

		line = StripBOM(line);
		auto raw = TrimW(line);
		if (raw.empty()) continue;               // 空行
		if (raw[0] == L'#') continue;            // # コメント行

		// CSV 風に分解（"..." 対応）
		auto fields = SplitCsvLineW(raw);
		if (fields.empty()) continue;

		// 必須 6～7 フィールド（Id, MenuStr, OnnxPath, ClsNamePath, YoloType, Resolution[, ClassCount]）
		if (fields.size() < 6) {
			// 足りない行はスキップ（必要ならログなど）
			continue;
		}

		MenuItemOnnx item{};
		try {
			item.Id = fields[0];
			item.MenuStr = fields[1];
			item.OnnxPath = fields[2];
			item.ClsNamePath = fields[3];
			item.YoloType = fields[4];

			// 数値系は stoi 前にトリムしておく
			item.Resolution = std::stoi(TrimW(fields[5]));
			if (fields.size() >= 7) {
				item.ClassCount = std::stoi(TrimW(fields[6]));
			}
			else {
				// 欠けていたら 0 / またはスキップでも可
				item.ClassCount = 0;
			}
			menu.push_back(std::move(item));
		}
		catch (...) {
			// 不正な数値等はこの行だけ無視
			continue;
		}
	}
	return 0;
}




//////////////////////////////////////////////////////////////////////
// メニューを構築する
int make_onnx_menus_by_id(
	HWND hWnd,
	UINT rootItemId,
	const std::vector<MenuItemOnnx>& menus,
	UINT IDM_START,
	const wchar_t* rootText /*表示名*/)
{
	HMENU hMenu = GetMenu(hWnd);
	if (!hMenu) return -1;

	// 新しいサブメニューを作る
	HMENU hSub = CreatePopupMenu();
	if (!hSub) return -1;

	// 子項目を追加
	for (size_t i = 0; i < menus.size(); ++i) {
		AppendMenuW(hSub, MF_STRING, IDM_START + static_cast<UINT>(i) + 1, menus[i].MenuStr.c_str());
	}

	// 既存の rootItemId を「ポップアップ」に置き換えてサブメニューをぶら下げる
	// ※ 第3引数に MF_POPUP を付け、uIDNewItem に HMENU を渡すのがポイント
	if (!ModifyMenuW(hMenu, rootItemId, MF_BYCOMMAND | MF_POPUP,
		(UINT_PTR)hSub, rootText ? rootText : L"Models")) {
		DestroyMenu(hSub);
		return -1;
	}

	DrawMenuBar(hWnd);
	return static_cast<int>(menus.size());
}
