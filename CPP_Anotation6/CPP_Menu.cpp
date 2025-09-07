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
// ---------- �����R�[�h�ϊ��i<codecvt> ��ˑ��j ----------
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
// ���j���[���t�@�C������ǂݍ���
/*
inline int loadmenu_onnx_old(const std::wstring& filename, std::vector<MenuItemOnnx>& menu)
{
	std::wifstream _ifs(filename);

	if (_ifs)
	{
		//�����R�[�h�̒���
		//_ifs.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>)); // ��UTF-8
		_ifs.imbue(std::locale(".932")); // ��CP932�iShift-JIS�j
		//_ifs.imbue(std::locale("ja_JP.SJIS"));

		std::wstring line;
		while (std::getline(_ifs, line))
		{
			if (line.empty())
				continue; // ��s�͖���
			if (line[0] == '#')
				continue; // �R�����g�s�͖���
			//��s��
			MenuItemOnnx item;
			size_t pos = 0;
			size_t comma_pos = 0;
			int field_index = 0;
			while ((comma_pos = line.find(',', pos)) != std::string::npos)
			{
				std::wstring field = line.substr(pos, comma_pos - pos);
				field.erase(0, field.find_first_not_of(L" \t")); // �O�̋󔒂��폜
				field.erase(field.find_last_not_of(L" \t") + 1); // ��̋󔒂��폜
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
						break; // �s�v�ȃt�B�[���h�͖���
				}
				pos = comma_pos + 1;
				field_index++;
			}
			// �Ō�̃t�B�[���h������
			if (pos < line.size() && field_index == 5)
			{
				std::wstring field = line.substr(pos);
				field.erase(0, field.find_first_not_of(L" \t")); // �O�̋󔒂��폜
				field.erase(field.find_last_not_of(L" \t") + 1); // ��̋󔒂��폜
				item.ClassCount = std::stoi(field);
			}
			if (field_index >= 5) // �K�v�ȃt�B�[���h�������Ă���ꍇ�̂ݒǉ�
			{
				menu.push_back(item);
			}
		}
		return 0; // ����

	}

	else
	{
		return -1; // �t�@�C�����J���Ȃ�
	}
}
*/

// �ǂ����w�b�_�̏�̕��iloadmenu_onnx ���O�j�ɒu��

// �t�@�C�����ۂ��Ɠǂݍ���� UTF-8�iBOM�t�����j or CP932 �� wstring �ɕϊ�
static inline std::wstring ReadTextFileW(const std::wstring& path)
{
	std::ifstream fs(path, std::ios::binary);
	if (!fs) return L"";

	std::vector<char> bin((std::istreambuf_iterator<char>(fs)), {});
	if (bin.empty()) return L"";

	// UTF-8 BOM ����
	UINT cp = 932; size_t off = 0;
	if (bin.size() >= 3 &&
		(unsigned char)bin[0] == 0xEF && (unsigned char)bin[1] == 0xBB && (unsigned char)bin[2] == 0xBF) {
		cp = CP_UTF8; off = 3;
	}

	auto in = bin.data() + off;
	auto len = (int)(bin.size() - off);

	// �܂�UTF-8�Ō����Ɏ����iBOM����͂����ɗ���j
	if (cp == CP_UTF8) {
		int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in, len, nullptr, 0);
		if (wlen > 0) {
			std::wstring w(wlen, 0);
			MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in, len, &w[0], wlen);
			return w;
		}
		// ���s������ CP932 �Ƀt�H�[���o�b�N
		cp = 932;
	}

	int wlen = MultiByteToWideChar(cp, 0, in, len, nullptr, 0);
	std::wstring w(wlen, 0);
	MultiByteToWideChar(cp, 0, in, len, &w[0], wlen);
	return w;
}

// �e�L�X�g�iwstring�j�� \r\n / \n / \r ���ׂĂōs����
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
// ���j���[���t�@�C������ǂݍ��ށi"..."�Ή��� / CSV���p�[�X�j
int loadmenu_onnx(const std::wstring& filename, std::vector<MenuItemOnnx>& menu)
{
	// �i�K�v�Ȃ������ Trim ���g����OK�j
	auto TrimW = [](const std::wstring& s) -> std::wstring {
		const auto b = s.find_first_not_of(L" \t\r\n");
		if (b == std::wstring::npos) return L"";
		const auto e = s.find_last_not_of(L" \t\r\n");
		return s.substr(b, e - b + 1);
		};

	// CSV 1 �s�� "�ōl���������i, �̓N�H�[�g�O�̂݋�؂� / "" �̓N�H�[�g���̃G�X�P�[�v�j
	auto SplitCsvLineW = [&](const std::wstring& src) -> std::vector<std::wstring> {
		std::vector<std::wstring> out;
		std::wstring field;
		bool inQuotes = false;

		for (size_t i = 0; i < src.size(); ++i) {
			wchar_t ch = src[i];
			if (ch == L'"') {
				if (inQuotes && i + 1 < src.size() && src[i + 1] == L'"') {
					// �A������ "" �̓G�X�P�[�v���ꂽ 1 �� " �Ƃ��Ċi�[
					field.push_back(L'"');
					++i;
				}
				else {
					inQuotes = !inQuotes; // �J�g�O��
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

	// UTF-8/UTF-16 BOM �������iSJIS �ǂ݂ł� wifstream �ɂ� U+FEFF �������邱�Ƃ�����j
	auto StripBOM = [](std::wstring s) {
		if (!s.empty() && s[0] == 0xFEFF) s.erase(0, 1);
		return s;
		};

	std::wifstream ifs(filename);
	if (!ifs) return -1;

	ifs.imbue(std::locale(".932")); // CP932�iShift-JIS�j

	std::wstring line;
	size_t lineno = 0;
	while (std::getline(ifs, line)) {
		++lineno;

		line = StripBOM(line);
		auto raw = TrimW(line);
		if (raw.empty()) continue;               // ��s
		if (raw[0] == L'#') continue;            // # �R�����g�s

		// CSV ���ɕ����i"..." �Ή��j
		auto fields = SplitCsvLineW(raw);
		if (fields.empty()) continue;

		// �K�{ 6�`7 �t�B�[���h�iId, MenuStr, OnnxPath, ClsNamePath, YoloType, Resolution[, ClassCount]�j
		if (fields.size() < 6) {
			// ����Ȃ��s�̓X�L�b�v�i�K�v�Ȃ烍�O�Ȃǁj
			continue;
		}

		MenuItemOnnx item{};
		try {
			item.Id = fields[0];
			item.MenuStr = fields[1];
			item.OnnxPath = fields[2];
			item.ClsNamePath = fields[3];
			item.YoloType = fields[4];

			// ���l�n�� stoi �O�Ƀg�������Ă���
			item.Resolution = std::stoi(TrimW(fields[5]));
			if (fields.size() >= 7) {
				item.ClassCount = std::stoi(TrimW(fields[6]));
			}
			else {
				// �����Ă����� 0 / �܂��̓X�L�b�v�ł���
				item.ClassCount = 0;
			}
			menu.push_back(std::move(item));
		}
		catch (...) {
			// �s���Ȑ��l���͂��̍s��������
			continue;
		}
	}
	return 0;
}




//////////////////////////////////////////////////////////////////////
// ���j���[���\�z����
int make_onnx_menus_by_id(
	HWND hWnd,
	UINT rootItemId,
	const std::vector<MenuItemOnnx>& menus,
	UINT IDM_START,
	const wchar_t* rootText /*�\����*/)
{
	HMENU hMenu = GetMenu(hWnd);
	if (!hMenu) return -1;

	// �V�����T�u���j���[�����
	HMENU hSub = CreatePopupMenu();
	if (!hSub) return -1;

	// �q���ڂ�ǉ�
	for (size_t i = 0; i < menus.size(); ++i) {
		AppendMenuW(hSub, MF_STRING, IDM_START + static_cast<UINT>(i) + 1, menus[i].MenuStr.c_str());
	}

	// ������ rootItemId ���u�|�b�v�A�b�v�v�ɒu�������ăT�u���j���[���Ԃ牺����
	// �� ��3������ MF_POPUP ��t���AuIDNewItem �� HMENU ��n���̂��|�C���g
	if (!ModifyMenuW(hMenu, rootItemId, MF_BYCOMMAND | MF_POPUP,
		(UINT_PTR)hSub, rootText ? rootText : L"Models")) {
		DestroyMenu(hSub);
		return -1;
	}

	DrawMenuBar(hWnd);
	return static_cast<int>(menus.size());
}
