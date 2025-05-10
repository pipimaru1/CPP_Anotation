#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFunctions.h"
#include "CPP_Anotation6.h"

// ������O��̋󔒍폜
static std::wstring Trim(const std::wstring& s) {
    const auto start = s.find_first_not_of(L" \t\r\n");
    if (start == std::wstring::npos) return L"";
    const auto end = s.find_last_not_of(L" \t\r\n");
    return s.substr(start, end - start + 1);
}

// DashStyle �� ������ �ϊ�
static Gdiplus::DashStyle StringToDashStyle(const std::wstring& str) {
    if (str == L"DashStyleSolid")       return Gdiplus::DashStyleSolid;
    if (str == L"DashStyleDash")        return Gdiplus::DashStyleDash;
    if (str == L"DashStyleDot")         return Gdiplus::DashStyleDot;
    if (str == L"DashStyleDashDot")     return Gdiplus::DashStyleDashDot;
    if (str == L"DashStyleDashDotDot")  return Gdiplus::DashStyleDashDotDot;
    if (str == L"DashStyleCustom")      return Gdiplus::DashStyleCustom;
    // �f�t�H���g
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
/// @brief �N���X���ސݒ�̓ǂݏ���
/// @param _filepath    ���o�̓t�@�C���p�X�i���C�h������j
/// @param _clsNames    �N���X���iwstring�j
/// @param _clsColors   �N���X�J���[�iGdiplus::Color�j
/// @param _dashStyles  ����iGdiplus::DashStyle�j
/// @param _clsPenWidths  �y�����iint�j
/// @param _rw          0=�ǂݍ��݁A1=��������
/// @return 0=�����A���l=�G���[
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
        // ��������
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
			return -1; // �t�@�C���I�[�v�����s
        }
    }
    else if (_rw == 0) 
    {
        // �ǂݍ���
        std::wifstream ifs(_filepath);
        if (!ifs) return -2; // �t�@�C���I�[�v�����s
        _clsNames.clear();
        _clsColors.clear();
        _dashStyles.clear();
        _clsPenWidths.clear();
		
        // 1�s���ǂݍ���
        std::wstring line;
        while (std::getline(ifs, line)) 
        {
            if (!line.empty()) //�f�[�^�������
            {
                std::wstringstream ss(line);
                std::wstring token;
                std::vector<std::wstring> tokens;
                // �J���}�ŋ�؂�B�p�[�X�Ƃ������B
                while (std::getline(ss, token, L','))
                {
                    tokens.push_back(Trim(token));
                }
                if (tokens.size() == 7)
                {
                    // �C���f�b�N�X�� tokens[0]�i�g��Ȃ��j
                    // tokens[1]: ����
                    _clsNames.push_back(tokens[1]);
                    // tokens[2-4]: R,G,B
                    int r = std::stoi(tokens[2]);
                    int g = std::stoi(tokens[3]);
                    int b = std::stoi(tokens[4]);
                    _clsColors.emplace_back(r, g, b);
                    // tokens[5]: DashStyle
                    _dashStyles.push_back(StringToDashStyle(tokens[5]));
                    // tokens[6]: �y����
                    _clsPenWidths.push_back(std::stoi(tokens[6]));
                }
                else // �f�[�^�����s��
                {
                    continue;
                }
            }
            else // ��s�̓X�L�b�v
            {
                continue;
            }
        }
    }
    else 
    {
        return -3; // _rw ��0/1�ȊO
    }
    return 0;
}
