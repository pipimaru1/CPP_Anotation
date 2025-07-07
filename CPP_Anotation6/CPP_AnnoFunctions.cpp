#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFunctions.h"
#include "CPP_Anotation6.h"

#pragma comment(lib, "Pathcch.lib")
#pragma comment(lib,"winmm.lib")

//Windows�̉�
wchar_t SOUND_ARRY[39][2][256] = {
    {L"NFP ����",                          L"Notification.Proximity" },
    {L"NFP �ڑ�",                          L"ProximityConnection" },
    {L"Windows �̏I��",                    L"SystemExit" },
    {L"Windows �̋N��",                    L"SystemStart" },
    {L"Windows �e�[�}�̕ύX",              L"ChangeTheme" },
    {L"Windows ���[�U�[ �A�J�E���g����",   L"WindowsUAC" },
    {L"Windows ���O�I�t",                  L"WindowsLogoff" },
    {L"Windows ���O�I��",                  L"WindowsLogon" },
    {L"�C���X�^���g ���b�Z�[�W�̒ʒm",     L"Notification.IM" },
    {L"�V�X�e�� �G���[",                   L"SystemHand" },
    {L"�V�X�e���ʒm",                      L"SystemNotification" },
    {L"�c�[�� �o�[ �o���h�̕\��",          L"ShowBand" },
    {L"�f�X�N�g�b�v ���[���̒ʒm",         L"MailBeep" },
    {L"�f�o�C�X�̐ؒf",                    L"DeviceDisconnect" },
    {L"�f�o�C�X�̐ڑ�",                    L"DeviceConnect" },
    {L"�f�o�C�X�̐ڑ��̎��s",              L"DeviceFail" },
    {L"�o�b�e���ቺ�A���[��",              L"LowBatteryAlarm" },
    {L"�o�b�e���؂�A���[��",              L"CriticalBatteryAlarm" },
    {L"�v���O���� �G���[",                 L"AppGPFault" },
    {L"�v���O�����̏I��",                  L"Close" },
    {L"�v���O�����̋N��",                  L"Open" },
    {L"���b�Z�[�W (�₢���킹)",           L"SystemQuestion" },
    {L"���b�Z�[�W (���)",                 L"SystemAsterisk" },
    {L"���b�Z�[�W (�x��)",                 L"SystemExclamation" },
    {L"���b�Z�[�W�̃V�F�C�N",              L"MessageNudge" },
    {L"���j���[ �R�}���h",                 L"MenuCommand" },
    {L"���j���[ �|�b�v�A�b�v",             L"MenuPopup" },
    {L"��ʂ̌x����",                      L".Default" },
    {L"�\��\�̃A���[��",                  L"Notification.Reminder" },
    {L"���ɖ߂� (�g��)",                   L"RestoreUp" },
    {L"���ɖ߂� (�k��)",                   L"RestoreDown" },
    {L"�������",                          L"PrintComplete" },
    {L"�V���e�L�X�g ���b�Z�[�W�̒ʒm",     L"Notification.SMS" },
    {L"�V���t�@�b�N�X�̒ʒm",              L"FaxBeep" },
    {L"�V�����[���̒ʒm",                  L"Notification.Mail" },
    {L"�ő剻",                            L"Maximize" },
    {L"�ŏ���",                            L"Minimize" },
    {L"�ʒm",                              L"Notification.Default" },
    {L"�I��",                              L"CCSelect" }
};



///////////////////////////////////////////////////////////////////////
// �摜�t�@�C�����ǂ����𔻒肷��֐�
bool IsImageFile(const std::wstring& fileName)
{
    for (const auto& pattern : GP.IMAGE_EXTENSIONS) {
        if (PathMatchSpecW(fileName.c_str(), pattern.c_str())) {
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////
// �t�H���_�̉摜�t�@�C�����擾����֐�
int GetImgsPaths(const std::wstring& folderPath, std::vector <ImgObject>& _imgObjs)
{
    _imgObjs.clear();
    _imgObjs.reserve(100);

    std::wstring searchPath = folderPath;
    if (!searchPath.empty() && searchPath.back() != L'\\')
        searchPath += L'\\';
    searchPath += L"*.*";  // �S�t�@�C���Ώ�

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0; // �t�H���_��������Ȃ�
    }

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wstring fileName = findData.cFileName;
            if (IsImageFile(fileName)) 
            {
				_imgObjs.emplace_back();//�v�f��ǉ�
				ImgObject& _imgobj = _imgObjs.back(); // �ǉ������v�f���Q��
                
                _imgobj.path = folderPath;

                if (!_imgobj.path.empty() && _imgobj.path.back() != L'\\')
                    _imgobj.path += L'\\';
                _imgobj.path += fileName;

                _imgobj.image = std::make_unique<Gdiplus::Image>(_imgobj.path.c_str());
                // ���s����̓|�C���^�� null �ł͂Ȃ� GDI+ �X�e�[�^�X��
                if (_imgobj.image->GetLastStatus() != Gdiplus::Ok) {
                    _imgobj.image = std::make_unique<Gdiplus::Image>(L"NO Image");
                }

                // ��`�̏�����
				_imgobj.objs.clear();
				_imgobj.objIdx = 0;
            }
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    return static_cast<int>(_imgObjs.size());
}
///////////////////////////////////////////////////////////////////////
// �t�H���_�̉摜�t�@�C������摜�f�[�^���擾����֐�
int LoadImageFiles(const std::wstring& folderPath, std::vector<ImgObject>& _imgObjs)
{
    _imgObjs.clear();
    _imgObjs.reserve(100);

    // �x�[�X�p�X���ɍ���Ă���
    std::wstring base = folderPath;
    if (!base.empty() && base.back() != L'\\') base += L'\\';
    std::wstring searchPath = base + L"*.*";

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    do {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            std::wstring fn = findData.cFileName;
            if (!IsImageFile(fn)) continue;

            // �@ �v�f��ǉ����ĎQ�Ƃ��擾
			//ImgObject& img = _imgObjs.emplace_back(); //release���ƃG���[�ɂȂ�
            // Replace the problematic line with the following code:
            _imgObjs.emplace_back(); // Add a new element
            ImgObject& img = _imgObjs.back(); // Get a reference to the newly added element
            // �A �p�X�ݒ�
            img.path = base + fn;

            // �B �摜�ǂݍ��݁iunique_ptr �ň��S�Ǘ��j
			// ���������[�N�h�~�̂��߁Aunique_ptr ���g�p
            img.image = std::make_unique<Gdiplus::Image>(img.path.c_str());

            // ���s����̓|�C���^�� null �ł͂Ȃ� GDI+ �X�e�[�^�X�ōs��
            if (img.image->GetLastStatus() != Gdiplus::Ok) 
            {
				// �摜�ǂݍ��ݎ��s���� NO Image ��\��
                img.image = std::make_unique<Gdiplus::Image>(L"NO Image");
            }

            // �C objs �̓f�t�H���g�ŋ�Ȃ̂� clear �s�v�AobjIdx ������ 0
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    return static_cast<int>(_imgObjs.size());
}
///////////////////////////////////////////////////////////////////////
// �t�H���_�̉摜�t�@�C������摜�f�[�^���擾����֐�
int LoadImageFilesMP(const std::wstring& folderPath, std::vector<ImgObject>& _imgObjs)
{
    _imgObjs.clear();

    // 1) �t�H���_���̃t�@�C������񋓂��ăp�X�����𒙂߂�
    std::wstring base = folderPath;
    if (!base.empty() && base.back() != L'\\') base += L'\\';
    std::wstring searchPath = base + L"*.*";

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    std::vector<std::wstring> fileList;
    do {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
            std::wstring fn = findData.cFileName;
            if (!IsImageFile(fn)) continue;
            fileList.push_back(base + fn);
        }
    } while (FindNextFileW(hFind, &findData));
    FindClose(hFind);

    const int N = static_cast<int>(fileList.size());
    if (N != 0)
    {
        // 2) ���ʃR���e�i�����炩���ߊm�ۂ��Ă���
        _imgObjs.resize(N);

        // 3) �摜���[�h�����
#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < N; ++i) {
            ImgObject& img = _imgObjs[i];
            img.path = fileList[i];

#define _RELEASE_IMAGE
#ifdef RELEASE_IMAGE
            auto imgFile = std::make_unique<Gdiplus::Image>(img.path.c_str());
            if (imgFile->GetLastStatus() == Gdiplus::Ok)
            {
                auto w = imgFile->GetWidth();
                auto h = imgFile->GetHeight();
                auto pf = imgFile->GetPixelFormat();

                auto bmp = std::make_unique<Gdiplus::Bitmap>(w, h, pf);
                Gdiplus::Graphics g(bmp.get());
                g.DrawImage(imgFile.get(), 0, 0, w, h);

                imgFile.reset();              // �����Ńn���h�����
                img.image = std::move(bmp); // �ȍ~�̓�������� Bitmap ��ێ�
            }
#else
            // unique_ptr �ŊǗ�
            auto image = std::make_unique<Gdiplus::Image>(img.path.c_str());
            if (image->GetLastStatus() != Gdiplus::Ok) {
                // ���[�h���s���͑�փC���[�W
                image = std::make_unique<Gdiplus::Image>(L"NO Image");
            }
            img.image = std::move(image);
#endif
        }
    }
    else
        return 0;
    return N;
}

///////////////////////////////////////////////////////////////////////
// ��`�̍��W�𐳋K������֐��@Gdiplus::RectF
// X, Y�F��`�̍�����̍��W,
// Width, Height�F��`�̕��ƍ���
void NormalizeRect(Gdiplus::RectF& r) 
{
    if (r.Width < 0) 
    {
        r.X += r.Width;
        r.Width = -r.Width;
    }
    if (r.Height < 0) 
    {
        r.Y += r.Height;
        r.Height = -r.Height;
    }

    if (r.X < 0)
        r.X = 0;
    if (r.Y < 0)
        r.Y = 0;
    if ((r.X + r.Width) > 1)
        r.Width = r.Width - ((r.X + r.Width) - 1);
    if ((r.Y + r.Height) > 1)
        r.Height = r.Height - ((r.Y + r.Height) - 1);
}

///////////////////////////////////////////////////////////////////////
// ��`�̍��W���X�P�[�����O����
void SscalingRect(Gdiplus::RectF& r_in, Gdiplus::RectF& r_out, float scaleX, float scaleY)
{
    // ����`�̒��S
    float cx = r_in.X + r_in.Width * 0.5f;
    float cy = r_in.Y + r_in.Height * 0.5f;

    // �V�������E����
    float newW = r_in.Width * scaleX;
    float newH = r_in.Height * scaleY;

    // ���S���Œ肵�č�������߂�
    r_out.X = cx - newW * 0.5f;
    r_out.Y = cy - newH * 0.5f;
    r_out.Width = newW;
    r_out.Height = newH;

    NormalizeRect(r_out);
}

///////////////////////////////////////////////////////////////////////
// �t�H���_�I���_�C�A���O��\������֐�
std::wstring GetFolderPath(HWND hWnd)
{
    std::wstring folderPath;
    IFileDialog* pFileDialog = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFileDialog));

    if (SUCCEEDED(hr)) {
        DWORD dwOptions = 0;
        pFileDialog->GetOptions(&dwOptions);
        pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);

        hr = pFileDialog->Show(hWnd);
        if (SUCCEEDED(hr)) {
            IShellItem* pItem = nullptr;
            hr = pFileDialog->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszFilePath = nullptr;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                if (SUCCEEDED(hr)) {
                    folderPath = pszFilePath;
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileDialog->Release();
    }
    return folderPath;
}
///////////////////////////////////////////////////////////////////////
// �t�H���_�I���_�C�A���O��\������֐�
// �R�[���o�b�N�֐��F�_�C�A���O�N�����ɏ����t�H���_��ݒ�
static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED)
    {
        // lpData �ɓn���������t�H���_��������g���đI����ݒ�
        SendMessage(hwnd, BFFM_SETSELECTIONW, TRUE, lpData);
    }
    return 0;
}

// �t�H���_�I���_�C�A���O�iSHBrowseForFolder�Łj
// hWnd           : �e�E�B���h�E�n���h��
// _currentFolder : �����\���t�H���_�̃p�X�i�󕶎��Ȃ����j
// _title         : �_�C�A���O�̃^�C�g���i�󕶎��Ȃ���胁�b�Z�[�W�j
std::wstring GetFolderPathEx(
    HWND hWnd,
    const std::wstring& _currentFolder,
    const std::wstring& _title)
{
    wchar_t szPath[MAX_PATH] = { 0 };

    BROWSEINFO bi = {};
    bi.hwndOwner = hWnd;
    bi.pszDisplayName = szPath;                  // �I�����ʂ̃o�b�t�@
    bi.lpszTitle = _title.c_str();          // �^�C�g�����w��
    bi.ulFlags = BIF_RETURNONLYFSDIRS     // �t�H���_�݂̂�Ԃ�
        | BIF_USENEWUI            // �V���� UI
        | BIF_BROWSEINCLUDEFILES; // �t�@�C�����c���[�ɕ\��
    bi.lpfn = BrowseCallbackProc;      // �����t�H���_�ݒ�p�R�[���o�b�N
    bi.lParam = reinterpret_cast<LPARAM>(
        _currentFolder.empty()
        ? nullptr
        : _currentFolder.c_str());

    PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);
    if (pidl)
    {
        if (SHGetPathFromIDList(pidl, szPath))
        {
            CoTaskMemFree(pidl);
            return std::wstring(szPath);
        }
        CoTaskMemFree(pidl);
    }
    return L"";  // �L�����Z�����Ȃ�
}
///////////////////////////////////////////////////////////////////////
// �t�H���_�I���_�C�A���O�iIFileDialog�Łj
// hWnd: �e�E�B���h�E
// _currentFolder: �����\���t�H���_�̃p�X�i�󕶎��Ȃ����t�H���_�j
// _title: �_�C�A���O�㕔�ɕ\������^�C�g���i�󕶎��Ȃ����^�C�g���j
std::wstring GetFolderPathIF(
    HWND hWnd,
    const std::wstring& _currentFolder,
    const std::wstring& _title)
{
    std::wstring result;

    // COM ������
    HRESULT hr = CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    IFileDialog* pfd = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr) && pfd)
    {
        // �t�H���_�I�����[�h�ɂ���
        DWORD dwOptions;
        pfd->GetOptions(&dwOptions);
        pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR);

        // �����t�H���_��ݒ�
        if (!_currentFolder.empty()){
            IShellItem* psiInit = nullptr;
            if (SUCCEEDED(SHCreateItemFromParsingName(_currentFolder.c_str(),
                nullptr,IID_PPV_ARGS(&psiInit))))
            {
                pfd->SetFolder(psiInit);
                psiInit->Release();
            }
        }

        // �^�C�g����ݒ�
        if (!_title.empty())
            pfd->SetTitle(_title.c_str());

        // �_�C�A���O�\��
        if (SUCCEEDED(pfd->Show(hWnd))){
            IShellItem* psiResult = nullptr;
            if (SUCCEEDED(pfd->GetResult(&psiResult))){
                PWSTR pszPath = nullptr;
                if (SUCCEEDED(psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))){
                    result = pszPath;
                    CoTaskMemFree(pszPath);
                }
                psiResult->Release();
            }
        }
        pfd->Release();
    }
    CoUninitialize();
    return result;
}

///////////////////////////////////////////////////////////////////////
// �t�H���_�I���_�C�A���O�iIFileDialog�Łj
// hWnd: �e�E�B���h�E
// _title: �_�C�A���O�㕔�ɕ\������^�C�g���i�󕶎��Ȃ����^�C�g���j
//���W�X�g���ɕۑ�����

std::wstring GetFolderPathIFR(
    HWND hWnd,
    const std::wstring& dlgTitle,
    const std::wstring& regValueName // ��Ȃ� dlgTitle ��l����
)
{
    std::wstring result;
    //const wchar_t* subKey = L"Software\\YourCompany\\YourApp\\FolderDialog";
    const wchar_t* subKey = REGSTRY_KEYSTRING_FOLDER;
    std::wstring valueName = regValueName.empty() ? dlgTitle : regValueName;

    // 1) ���W�X�g������O��̃t�H���_��ǂݏo��
    std::wstring initialFolder;
    HKEY hKey = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, subKey, 0, nullptr,
        REG_OPTION_NON_VOLATILE, KEY_READ, nullptr,
        &hKey, nullptr) == ERROR_SUCCESS)
    {
        wchar_t buf[MAX_PATH] = { 0 };
        DWORD bufSize = sizeof(buf), type = REG_SZ;
        if (RegQueryValueExW(hKey,
            valueName.c_str(),
            nullptr,
            &type,
            reinterpret_cast<BYTE*>(buf),
            &bufSize) == ERROR_SUCCESS)
        {
            initialFolder = buf;
        }
        RegCloseKey(hKey);
    }

    // 2) COM ������ �� IFileDialog �쐬
    HRESULT hr = CoInitializeEx(nullptr,
        COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) return L"";

    IFileDialog* pfd = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr) && pfd)
    {
        // �t�H���_�I�����[�h��
        DWORD opts = 0;
        pfd->GetOptions(&opts);
        pfd->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

        // �����t�H���_
        if (!initialFolder.empty())
        {
            IShellItem* psiInit = nullptr;
            if (SUCCEEDED(SHCreateItemFromParsingName(
                initialFolder.c_str(),
                nullptr,
                IID_PPV_ARGS(&psiInit))))
            {
                pfd->SetFolder(psiInit);
                psiInit->Release();
            }
        }

        // �^�C�g���ݒ�
        if (!dlgTitle.empty())
            pfd->SetTitle(dlgTitle.c_str());

        // �_�C�A���O�\��
        if (SUCCEEDED(pfd->Show(hWnd)))
        {
            IShellItem* psiRes = nullptr;
            if (SUCCEEDED(pfd->GetResult(&psiRes)))
            {
                PWSTR pszPath = nullptr;
                if (SUCCEEDED(psiRes->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)))
                {
                    result = pszPath;
                    CoTaskMemFree(pszPath);

                    // 3) ���ʂ����W�X�g���ɕۑ�
                    HKEY hKey2 = nullptr;
                    if (RegCreateKeyExW(HKEY_CURRENT_USER, subKey, 0, nullptr,
                        REG_OPTION_NON_VOLATILE, KEY_WRITE,
                        nullptr, &hKey2, nullptr) == ERROR_SUCCESS)
                    {
                        RegSetValueExW(hKey2,
                            valueName.c_str(),
                            0,
                            REG_SZ,
                            reinterpret_cast<const BYTE*>(result.c_str()),
                            static_cast<DWORD>((result.size() + 1) * sizeof(wchar_t)));
                        RegCloseKey(hKey2);
                    }
                }
                psiRes->Release();
            }
        }

        pfd->Release();
    }
    CoUninitialize();
    return result;
}


///////////////////////////////////////////////////////////////////////
// LabelObj���t�@�C���ۑ����邽�߂̕����񐶐��֐�
// ���͒l��LabelObj
// �o�͒l�͕����� std::wstring
// UTF-8�ŕۑ�����
// YOLO�`���ŕۑ�����
// wchar_t �� UTF-8 �̕ϊ��iC++17 �̏ꍇ�j
std::string LabelsToString(
    const LabelObj& obj, 
	int mode, // 0:default, 1:Yolo
	int _sc, // 0:���K���A1:�X�P�[�����ꂽ��`���g�p
	float minimumsize // �ŏ��T�C�Y�����i�f�t�H���g�͂Ȃ��j
)
{
    std::ostringstream oss;
    Gdiplus::RectF _rct;
    if (_sc != 0 ) 
        _rct = obj.Rct_Scale; // �X�P�[�����ꂽ��`���g�p
    else
		_rct = obj.Rct; // ���K�����ꂽ��`���g�p

	if (minimumsize > 0){
        if (GP.isMinimumLabelCrrect){ //�ŏ��T�C�Y�␳
            if (obj.Rct.Width < minimumsize)
            {
				_rct.X = obj.Rct.X - (minimumsize - obj.Rct.Width) / 2; // ������
                _rct.Width = minimumsize; // �ŏ�������
            }
            if (obj.Rct.Height < minimumsize)
            {
				_rct.Y = obj.Rct.Y - (minimumsize - obj.Rct.Height) / 2; // ������
                _rct.Height = minimumsize; // �ŏ���������
            }
        }else{
            // �ŏ��T�C�Y������K�p ���K�����ꂽ��`�Ŕ���
            if (obj.Rct.Width < minimumsize || obj.Rct.Height < minimumsize){
                return ""; // �T�C�Y������������ꍇ�͋󕶎����Ԃ��Ă����܂�
            }
        }
	}

    // ���l�ƃX�y�[�X�����Ȃ̂ŁA���ʂ� narrow string �� OK
    if (mode == 0)
    {
        oss << obj.ClassNum << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.X) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.Y) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.Width) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.Height);
    }
	else if (mode == 1)
	{
        oss << obj.ClassNum << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.X + _rct.Width /2) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.Y + _rct.Height/2) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.Width) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(_rct.Height);
	}
	else
	{   
		// �G���[����
		return "";
	}
    return oss.str();  // �Ԃ�l�� std::string �� UTF-8 �G���R�[�h�ς�
}

///////////////////////////////////////////////////////////////////////
// LabelObj�̕�������t�@�C���ۑ�����֐�
// ���͒l�̓t�@�C������const std::vector<LabelObj>&
// �o�͒l�͐���������true�A���s������false
bool SaveLabelsToFile(
    const std::wstring& fileName, 
    const std::vector<LabelObj>& objs, 
    int _sc,
	float minimumsize, // �ŏ��T�C�Y�����i�f�t�H���g�͂Ȃ��j
	int mode // 0:default, 1:yolo
){
	// UTF-8�ŕۑ����邽�߂̐ݒ�
	std::ofstream file(fileName, std::ios::binary);

	// �t�@�C���I�[�v��
	if (!file.is_open()) {
		return false; // �t�@�C���I�[�v�����s
	}
	for (const auto& obj : objs) {
		std::string _sc_str = LabelsToString(obj, mode, _sc, minimumsize);
		if (_sc_str.empty()) 
            continue; // �ŏ��T�C�Y�����Ɉ������������ꍇ�̓X�L�b�v
        else
			file << _sc_str << std::endl; // �X�P�[�����ꂽ��`���g�p
		//file << LabelsToString(obj, mode, minimumsize, _sc) << std::endl;
	}
	file.close();
	return true;
}

///////////////////////////////////////////////////////////////////////
// LabelObj�̕�������t�@�C���ۑ�����֐�
// ���͒l�̓t�@�C������const std::vector<LabelObj>&
// �o�͒l�͐���������true�A���s������false
//bool SaveLabelsToFileWithScale(
//    const std::wstring& fileName,
//    const std::vector<LabelObj>& objs,
//	float scaleX = 1.0f, // X���̃X�P�[��
//	float scaleY = 1.0f, // Y���̃X�P�[��
//    int mode = 0 // 0:default, 1:yolo
//) {
//    // UTF-8�ŕۑ����邽�߂̐ݒ�
//    std::ofstream file(fileName, std::ios::binary);
//
//    // �t�@�C���I�[�v��
//    if (!file.is_open()) {
//        return false; // �t�@�C���I�[�v�����s
//    }
//    for (const auto& obj : objs) {
//        file << LabelsToString(obj, mode, true) << std::endl;
//    }
//    file.close();
//    return true;
//}
//

///////////////////////////////////////////////////////////////////////
std::wstring get_now_time_string()
{
    // ���ݎ����� SYSTEMTIME �Ŏ擾
    SYSTEMTIME st;
    GetLocalTime(&st);

    // std::tm �ɕϊ�
    std::tm tm{};
    tm.tm_year = st.wYear - 1900;  // �N�� 1900 �N�_
    tm.tm_mon = st.wMonth - 1;     // ���� 0-11
    tm.tm_mday = st.wDay;
    tm.tm_hour = st.wHour;
    tm.tm_min = st.wMinute;
    tm.tm_sec = st.wSecond;
    tm.tm_isdst = -1; // �Ď��Ԃ̎�������
    // std::mktime �Ń^�C���X�^���v�ɕϊ�
    //std::time_t now = std::mktime(&tm);
    // std::put_time ���g���ăt�H�[�}�b�g
    std::wostringstream oss;
    oss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S");
    return oss.str(); // std::wstring �ŕԂ�
}

///////////////////////////////////////////////////////////////////////
// LabelObj�̕�������t�@�C���ۑ�����֐�
// �摜�y�[�W���O�̎��Ɏg��
bool SaveLabelsToFileSingle(HWND hWnd, size_t _idx, float minimumsize) // �ŏ��T�C�Y�����i�f�t�H���g�͂Ȃ��j
{
    if (GP.imgObjs[GP.imgIdx].isEdited) {
        // �t�@�C����
        std::wstring _fileName1;
        std::wstring _fileName2;
        _fileName1 = GetOnlyFileNameFormPath(GP.imgObjs[GP.imgIdx].path);
        _fileName2 = GP.labelFolderPath + L"\\" + _fileName1 + L".txt";
        bool _ret = SaveLabelsToFile(_fileName2, GP.imgObjs[GP.imgIdx].objs, 0, minimumsize, 1);

        if (_ret) {
            //�ҏW�t���O�����Z�b�g
            GP.imgObjs[GP.imgIdx].isEdited = false;
            //MessageBeep(0x010L);  // MB_OK�́u�|���v�Ƃ����W�����i�N���b�N���ɋ߂��j
            PlaySound(SOUND_ARRY[15][1], NULL, SND_ALIAS | SND_ASYNC | SND_NODEFAULT); // �炷  

            //�������񂾃t�@�C������"records.log"�ɏ�������
            //�����A�t�@�C�����A���x���̐�
            std::wstring logFile = GP.labelFolderPath + L"\\records.log";
            std::wofstream logStream(logFile, std::ios::app);
            if (logStream.is_open()) {
                SYSTEMTIME st;
                GetLocalTime(&st);
                logStream << get_now_time_string() << L" "
                    << L"ANNOTATIONS:" << GP.imgObjs[GP.imgIdx].objs.size() << L" "
                    << GP.labelFolderPath + L"\\" + _fileName1 << std::endl;
                logStream.close();
            }

            return true;
        }
        else {
            // �ۑ����s
            MessageBox(hWnd, L"�ۑ����s", L"���s", MB_OK);
            return false;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////
// �t�@�C�������_�C�A���O�{�b�N�X�Ŏ擾����֐�
// ���͒l�̓E�B���h�E�n���h��
// �o�͒l�̓t�@�C���� std::wstring
std::wstring GetFileName_old(HWND hWnd)
{
	std::wstring fileName;
	IFileDialog* pFileDialog = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pFileDialog));
	if (SUCCEEDED(hr)) {
		// �t�B���^��ݒ�
		COMDLG_FILTERSPEC filter[] = {
			{ L"YOLO", L"*.txt" },
			{ L"All Files", L"*.*" }
		};
		pFileDialog->SetFileTypes(ARRAYSIZE(filter), filter);
		// �_�C�A���O��\��
		hr = pFileDialog->Show(hWnd);
		if (SUCCEEDED(hr)) {
			IShellItem* pItem = nullptr;
			hr = pFileDialog->GetResult(&pItem);
			if (SUCCEEDED(hr)) {
				PWSTR pszFilePath = nullptr;
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
				if (SUCCEEDED(hr)) {
					fileName = pszFilePath;
					CoTaskMemFree(pszFilePath);
				}
				pItem->Release();
			}
		}
		pFileDialog->Release();
	}
	return fileName;
}

///////////////////////////////////////////////////////////////////////
// �t�@�C�������_�C�A���O�{�b�N�X�Ŏ擾����֐�
// ���͒l�̓E�B���h�E�n���h��
// �o�͒l�̓t�@�C���� std::wstring
// �t�@�C���I���_�C�A���O�{���W�X�g���ۑ�
std::wstring GetFileName(HWND hWnd, const std::wstring& title, int _rw)
{
    std::wstring fileName;

    // COM�_�C�A���O�쐬
    IFileDialog* pFileDialog = nullptr;

    // �_�C�A���O��ʂ�؂�ւ�
    const CLSID clsid = (_rw == 1)
        ? CLSID_FileSaveDialog
        : CLSID_FileOpenDialog;


    HRESULT hr = CoCreateInstance(
        clsid,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFileDialog)
    );
    if (FAILED(hr)) return L"";

    // �^�C�g���ݒ�
    pFileDialog->SetTitle(title.c_str());

    // �O��̃t�H���_�����W�X�g������ǂݏo��
    {
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, REGSTRY_KEYSTRING_FILE, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            wchar_t lastPath[MAX_PATH] = {};
            DWORD cb = sizeof(lastPath);
            if (RegQueryValueExW(hKey, title.c_str(), nullptr, nullptr, (BYTE*)lastPath, &cb) == ERROR_SUCCESS) {
                IShellItem* pFolder = nullptr;
                if (SUCCEEDED(SHCreateItemFromParsingName(lastPath, nullptr, IID_PPV_ARGS(&pFolder)))) {
                    pFileDialog->SetFolder(pFolder);
                    pFolder->Release();
                }
            }
            RegCloseKey(hKey);
        }
    }

    // �t�B���^�ݒ�
    COMDLG_FILTERSPEC filter[] = {
        { L"YOLO",      L"*.txt" },
        { L"All Files", L"*.*"   }
    };
    pFileDialog->SetFileTypes(ARRAYSIZE(filter), filter);

    // �_�C�A���O�\��
    hr = pFileDialog->Show(hWnd);
    if (SUCCEEDED(hr)) {
        IShellItem* pItem = nullptr;
        hr = pFileDialog->GetResult(&pItem);
        if (SUCCEEDED(hr)) {
            PWSTR pszFilePath = nullptr;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
            if (SUCCEEDED(hr) && pszFilePath) {
                fileName = pszFilePath;
                CoTaskMemFree(pszFilePath);
            }
            pItem->Release();
        }

        // �I�����ꂽ�t�@�C���̃t�H���_�����𒊏o
        if (!fileName.empty()) {
            std::wstring folder = fileName;
            // PathCchRemoveFileSpec �Ŗ����̃t�@�C����������
            PathCchRemoveFileSpec(&folder[0], folder.size());
            // ���W�X�g���ɕۑ��i�L�[��������΍쐬�j
            HKEY hKey = nullptr;
            if (RegCreateKeyExW(
                HKEY_CURRENT_USER,
                REGSTRY_KEYSTRING_FILE,
                0, nullptr,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                nullptr,
                &hKey,
                nullptr) == ERROR_SUCCESS)
            {
                RegSetValueExW(
                    hKey,
                    title.c_str(),
                    0, REG_SZ,
                    (const BYTE*)folder.c_str(),
                    static_cast<DWORD>((folder.size() + 1) * sizeof(wchar_t))
                );
                RegCloseKey(hKey);
            }
        }
    }

    pFileDialog->Release();
    return fileName;
}

///////////////////////////////////////////////////////////////////////
// �t�@�C���p�X����g���q���w��̊g���q�ɕύX����֐�
// ���͒l�̓t�@�C���p�X�Ɗg���q
// �o�͒l�͕ύX��̃t�@�C���p�X std::wstring
std::wstring ChangeFileExtension(const std::wstring& filePath, const std::wstring& newExt)
{
	std::wstring newFilePath = filePath;
	size_t pos = newFilePath.find_last_of(L'.');
	if (pos != std::wstring::npos) {
		newFilePath.erase(pos);
	}
	newFilePath += newExt;
	return newFilePath;
}
///////////////////////////////////////////////////////////////////////
// �t�@�C���p�X����g���q����菜���֐�
// ���͒l�̓t�@�C���p�X
// �o�͒l�͊g���q����菜�����t�@�C���p�X std::wstring
std::wstring RemoveFileExtension(const std::wstring& filePath)
{
	std::wstring newFilePath = filePath;
	size_t pos = newFilePath.find_last_of(L'.');
	if (pos != std::wstring::npos) {
		newFilePath.erase(pos);
	}
	return newFilePath;
}

///////////////////////////////////////////////////////////////////////
// �t�@�C���p�X����t�@�C�����̂ݒ��o����֐� 
// �g���q���폜
// ���͒l�̓t�@�C���p�X
// �o�͒l�̓t�@�C���� std::wstring
std::wstring GetOnlyFileNameFormPath(const std::wstring& filePath)
{
    std::wstring _fileName1;
    std::wstring _fileName2;
	size_t pos = filePath.find_last_of(L'\\');
	if (pos != std::wstring::npos) {
        _fileName1 = filePath.substr(pos + 1);
	}
	_fileName2 = RemoveFileExtension(_fileName1);

	return _fileName2;
}
///////////////////////////////////////////////////////////////////////
// �t�@�C���p�X����t�@�C�����̂ݒ��o����֐� �g���q�͎c��
// ���͒l�̓t�@�C���p�X
// �o�͒l�̓t�@�C���� std::wstring
std::wstring GetFileNameFormPath(const std::wstring& filePath)
{
    std::wstring _fileName1;
    size_t pos = filePath.find_last_of(L'\\');
    if (pos != std::wstring::npos) {
        _fileName1 = filePath.substr(pos + 1);
    }
    return _fileName1; // �g���q���܂ރt�@�C������Ԃ�
}



///////////////////////////////////////////////////////////////////////
//�t�H���_�p�X�ƃt�@�C�����Ɗg���q���w�肵��
//�t�@�C�������݂���΁A���̃t�@�C���̃t���p�X��Ԃ��֐�
//�t�@�C�������݂��Ȃ���΁A�󕶎�""��Ԃ�
std::wstring ChkFileExistWithPathExt(
    const std::wstring& folderpath, 
    const std::wstring& filename,
	const std::wstring& ext) // ".txt"�ȂǁB"."���܂ނ���
{
	std::wstring _fn = GetOnlyFileNameFormPath(filename);

	std::wstring fullPath = folderpath + L"\\" + _fn + ext;
	if (PathFileExistsW(fullPath.c_str())) 
    {
		return fullPath; // �t�@�C�������݂���ꍇ�A�t���p�X��Ԃ�
	}
	return L""; // �t�@�C�������݂��Ȃ��ꍇ�A�󕶎���Ԃ�
}

///////////////////////////////////////////////////////////////////////
// �֐� LoadLabels_to_Objects(
// ImgObject& imgObj, //�f�[�^���i�[����ImgObject�N���X�̎Q��
// const std::wstring& folderpath, //�A�m�e�[�V�����t�@�C���̂���t�H���_�p�X
// const std::wstring& ext //�A�m�e�[�V�����t�@�C���̊g���q
// )
int LoadLabels_to_Objects(
ImgObject& imgObj, //�f�[�^���i�[����ImgObject�N���X�̎Q��
const std::wstring& folderpath, //�A�m�e�[�V�����t�@�C���̂���t�H���_�p�X
const std::wstring& ext, //�A�m�e�[�V�����t�@�C���̊g���q
int mode //0:default, 1:yolo
){
	//�A�m�e�[�V�����t�@�C���̃t���p�X���擾
	std::wstring _fileName = ChkFileExistWithPathExt(folderpath, imgObj.path, ext);
	if (_fileName.empty()) 
    {
		return 0; // �t�@�C�������݂��Ȃ��ꍇ�A0��Ԃ�
	}
	
    //�A�m�e�[�V�����t�@�C�����J��
	std::wifstream file(_fileName);
	if (!file.is_open()) 
    {
		return 0; // �t�@�C���I�[�v�����s
	}

	//�A�m�e�[�V�����f�[�^��ǂݍ���
	imgObj.objs.clear(); // �����̃f�[�^���N���A
    if (mode == 0)
    {
        LabelObj obj;
        while (file >> obj.ClassNum >> obj.Rct.X >> obj.Rct.Y >> obj.Rct.Width >> obj.Rct.Height)
        {

            imgObj.objs.push_back(obj);
        }
    }
	else if (mode == 1)
	{
		float tmp_x, tmp_y, tmp_w, tmp_h;
        LabelObj obj;
		// YOLO�`���̃f�[�^��ǂݍ���
		// YOLO�`���́A�N���X�ԍ��Ax_center�Ay_center�Awidth�Aheight�̏�
		// x_center�Ay_center�Awidth�Aheight�͉摜�T�C�Y�Ŋ������l
		// �摜�T�C�Y�Ŋ������l�����ɖ߂����߂ɁAimgObj.width��imgObj.height���g�p
		while (file >> obj.ClassNum >> tmp_x >> tmp_y >> tmp_w >> tmp_h)
		{
            obj.Rct.X = tmp_x - tmp_w / 2;
			obj.Rct.Y = tmp_y - tmp_h / 2;
			obj.Rct.Width = tmp_w;
            obj.Rct.Height = tmp_h;
			// YOLO�`���̃f�[�^��LabelObj�`���ɕϊ�
			imgObj.objs.push_back(obj);
		}
	}
	file.close();
	return static_cast<int>(imgObj.objs.size());
}

///////////////////////////////////////////////////////////////////////
// �֐� LoadLabelFiles(
// std::vector<ImgObject>& imgObjs, //�f�[�^���i�[����ImgObject�N���X�̎Q��
// const std::wstring& folderpath, //�A�m�e�[�V�����t�@�C���̂���t�H���_�p�X
// const std::wstring& ext //�A�m�e�[�V�����t�@�C���̊g���q
// )
int LoadLabelFiles(
	std::vector<ImgObject>& imgObjs, //�f�[�^���i�[����ImgObject�N���X�̎Q��
	const std::wstring& folderpath, //�A�m�e�[�V�����t�@�C���̂���t�H���_�p�X
	const std::wstring& ext, //�A�m�e�[�V�����t�@�C���̊g���q
    int mode //0:default, 1:yolo
){
	int loadCount = 0; // �ǂݍ��񂾃A�m�e�[�V�����̐�
	for (int i = 0; i < imgObjs.size(); i++)
	{
        //�A�m�e�[�V�����t�@�C���̃t���p�X���擾
        std::wstring _fileName = ChkFileExistWithPathExt(folderpath, imgObjs[i].path, ext);
        if (_fileName.empty())
        {
			continue; // �t�@�C�������݂��Ȃ��ꍇ�A���̉摜��
        }
        //�A�m�e�[�V�����t�@�C�����J��
        std::wifstream file(_fileName);
        if (!file.is_open())
        {
            continue; // �t�@�C���I�[�v�����s
        }
        imgObjs[i].objs.clear(); // �����̃f�[�^���N���A
        
        
        if (mode == 0)
        {
            LabelObj obj;
            while (file >> obj.ClassNum >> obj.Rct.X >> obj.Rct.Y >> obj.Rct.Width >> obj.Rct.Height)
            {
				NormalizeRect(obj.Rct); // ��`�̍��W�𐳋K��

                if (obj.ClassNum < GP.ClsNames.size())
                    obj.ClassName = GP.ClsNames[obj.ClassNum];

                if (obj.ClassNum < GP.ClsColors.size())
                    obj.color = GP.ClsColors[obj.ClassNum];

                if (obj.ClassNum < GP.ClsDashStyles.size())
                    obj.dashStyle = GP.ClsDashStyles[obj.ClassNum];

                if (obj.ClassNum < GP.ClsPenWidths.size())
                    obj.penWidth = GP.ClsPenWidths[obj.ClassNum];

                imgObjs[i].objs.push_back(obj);
            }
        }
        else if (mode == 1)
        {
			float tmp_x, tmp_y, tmp_w, tmp_h;
			LabelObj obj;
			// YOLO�`���̃f�[�^��ǂݍ���
			// YOLO�`���́A�N���X�ԍ��Ax_center�Ay_center�Awidth�Aheight�̏�
			// x_center�Ay_center�Awidth�Aheight�͉摜�T�C�Y�Ŋ������l
			// �摜�T�C�Y�Ŋ������l�����ɖ߂����߂ɁAimgObj.width��imgObj.height���g�p
			while (file >> obj.ClassNum >> tmp_x >> tmp_y >> tmp_w >> tmp_h)
			{
				obj.Rct.X = tmp_x - tmp_w / 2;
				obj.Rct.Y = tmp_y - tmp_h / 2;
				obj.Rct.Width = tmp_w;
				obj.Rct.Height = tmp_h;

                NormalizeRect(obj.Rct); // ��`�̍��W�𐳋K��
                
                if (obj.ClassNum < GP.ClsNames.size())
					obj.ClassName = GP.ClsNames[obj.ClassNum];
				if (obj.ClassNum < GP.ClsColors.size())
					obj.color = GP.ClsColors[obj.ClassNum];
				if (obj.ClassNum < GP.ClsDashStyles.size())
					obj.dashStyle = GP.ClsDashStyles[obj.ClassNum];
				if (obj.ClassNum < GP.ClsPenWidths.size())
					obj.penWidth = GP.ClsPenWidths[obj.ClassNum];
				imgObjs[i].objs.push_back(obj);
			}
        }
        else
			continue;// �������Ȃ�

        file.close();
        loadCount++;
    }
    return loadCount;
}

int LoadLabelFilesMP(
    std::vector<ImgObject>& imgObjs,
    const std::wstring& folderpath,
    const std::wstring& ext,
    int mode
){
    int loadCount = 0;
    const int N = static_cast<int>(imgObjs.size());

    // OpenMP ���񉻁F�e i �͓Ɨ������Ȃ̂ŋ����Ȃ�
#pragma omp parallel for schedule(dynamic) reduction(+:loadCount)
    for (int i = 0; i < N; ++i)
    {
        // �t�@�C�����擾�i�X���b�h���ƂɃ��[�J���ȕϐ��j
        std::wstring _fileName = ChkFileExistWithPathExt(folderpath, imgObjs[i].path, ext);
        if (_fileName.empty()) continue;

        // �t�@�C�����J���ăp�[�X
        std::wifstream file(_fileName);
        if (!file.is_open()) continue;

        // �����f�[�^���N���A
        imgObjs[i].objs.clear();

        if (mode == 0)
        {
            LabelObj obj;
            while (file >> obj.ClassNum
                >> obj.Rct.X >> obj.Rct.Y
                >> obj.Rct.Width >> obj.Rct.Height)
            {
                NormalizeRect(obj.Rct);

                if (obj.ClassNum < GP.ClsNames.size())     obj.ClassName = GP.ClsNames[obj.ClassNum];
                if (obj.ClassNum < GP.ClsColors.size())    obj.color = GP.ClsColors[obj.ClassNum];
                if (obj.ClassNum < GP.ClsDashStyles.size())obj.dashStyle = GP.ClsDashStyles[obj.ClassNum];
                if (obj.ClassNum < GP.ClsPenWidths.size()) obj.penWidth = GP.ClsPenWidths[obj.ClassNum];

                imgObjs[i].objs.push_back(obj);
            }
        }
        else if (mode == 1)
        {
            float tmp_x, tmp_y, tmp_w, tmp_h;
            LabelObj obj;
            while (file >> obj.ClassNum >> tmp_x >> tmp_y >> tmp_w >> tmp_h)
            {
                // YOLO->BBox
                obj.Rct.X = tmp_x - tmp_w / 2;
                obj.Rct.Y = tmp_y - tmp_h / 2;
                obj.Rct.Width = tmp_w;
                obj.Rct.Height = tmp_h;
                NormalizeRect(obj.Rct);

                if (obj.ClassNum < GP.ClsNames.size())     obj.ClassName = GP.ClsNames[obj.ClassNum];
                if (obj.ClassNum < GP.ClsColors.size())    obj.color = GP.ClsColors[obj.ClassNum];
                if (obj.ClassNum < GP.ClsDashStyles.size())obj.dashStyle = GP.ClsDashStyles[obj.ClassNum];
                if (obj.ClassNum < GP.ClsPenWidths.size()) obj.penWidth = GP.ClsPenWidths[obj.ClassNum];

                imgObjs[i].objs.push_back(obj);
            }
        }
        // mode ������ȊO�Ȃ牽�����Ȃ�
        file.close();

        // ���̉摜�͐����ǂݍ��݂Ƃ݂Ȃ��ăJ�E���g
        loadCount++;
    }

    return loadCount;
}

///////////////////////////////////////////////////////////////////////
//��`�̐���Ƀ}�E�X�J�[�\�������邩�ǂ����𔻒肷��֐�
EditMode IsMouseOnRectEdge(
    const POINT& pt, 
    const LabelObj& obj,
    int overlap
){
	bool Left = false;
	bool Right = false;
	bool Top = false;
	bool Bottom = false;

    // ��`�̍��W
    float x0 = obj.Rct.X * GP.width;
    float y0 = obj.Rct.Y * GP.height;
    float w = obj.Rct.Width * GP.width;
    float h = obj.Rct.Height * GP.height;

    // �e�ӂ̕���я�Ɍ����ĂĔ���
	EditMode  _ret = EditMode::None;
    if(pt.y >= y0 - overlap && pt.y <= y0 + overlap &&
        pt.x >= x0 - overlap && pt.x <= x0 + w + overlap)
		Top = true; // ���

    if(pt.y >= y0 + h - overlap && pt.y <= y0 + h + overlap &&
        pt.x >= x0 - overlap && pt.x <= x0 + w + overlap)
		Bottom = true; // ����

    if(pt.x >= x0 - overlap && pt.x <= x0 + overlap &&
        pt.y >= y0 - overlap && pt.y <= y0 + h + overlap)
		Left = true; // ����

    if(pt.x >= x0 + w - overlap && pt.x <= x0 + w + overlap &&
        pt.y >= y0 - overlap && pt.y <= y0 + h + overlap)
		Right = true; // �E��

    if (Top && Left)
        return EditMode::LeftTop; // ��Ӎ��[
	if (Bottom && Left)
		return EditMode::LeftBottom; // ���Ӎ��[
	if (Top && Right)
		return EditMode::RightTop; // ��ӉE�[
	if (Bottom && Right)
		return EditMode::RightBottom; // ���ӉE�[
	if (Top)
		return EditMode::Top; // ���
	if (Bottom)
		return EditMode::Bottom; // ����
	if (Left)
		return EditMode::Left; // ����
	if (Right)
		return EditMode::Right; // �E��

	return EditMode::None; // �O 
}
///////////////////////////////////////////////////////////////////////
//��`�̐���Ƀ}�E�X�J�[�\�������邩�ǂ����𔻒肷��֐�
int IsMouseOnRectEdge_old(
    const POINT& pt,
    const LabelObj& obj,
    int overlap
){
    // ��`�̍��W
    float x0 = obj.Rct.X * GP.width;
    float y0 = obj.Rct.Y * GP.height;
    float w = obj.Rct.Width * GP.width;
    float h = obj.Rct.Height * GP.height;

    // �e�ӂ̕���я�Ɍ����ĂĔ���
    int _ret = 0;
    if (pt.y >= y0 - overlap && pt.y <= y0 + overlap &&
        pt.x >= x0 - overlap && pt.x <= x0 + w + overlap)
        return 1; // ���

    if (pt.y >= y0 + h - overlap && pt.y <= y0 + h + overlap &&
        pt.x >= x0 - overlap && pt.x <= x0 + w + overlap)
        return 2; // ����

    if (pt.x >= x0 - overlap && pt.x <= x0 + overlap &&
        pt.y >= y0 - overlap && pt.y <= y0 + h + overlap)
        return 3; // ����

    if (pt.x >= x0 + w - overlap && pt.x <= x0 + w + overlap &&
        pt.y >= y0 - overlap && pt.y <= y0 + h + overlap)
        return 4; // �E��

    return 0; // �O
}

///////////////////////////////////////////////////////////////////////
//�}�E�X�J�[�\���Əd�Ȃ��`�̃C���f�b�N�X���擾����֐�
//�ŏ��̈������Ԃ�
//�d�Ȃ�����`�̃I�u�W�F�N�g�ɂ�_mOver��1�`8�̒l������
size_t GetIdxMouseOnRectEdge(
    const POINT& pt,
    std::vector<LabelObj>& objs,
	EditMode& editMode,
    int overlap
){
    int _idx=-1;

	//�S���̋�`�̃}�E�X�I�[�o�[��Ԃ�����
    for (size_t i = 0; i < objs.size(); i++)
    //for (size_t i = 0; i < _obj_size; i++)
            objs[i].mOver = false;

    for (size_t i = 0; i < objs.size(); i++)
    {
        //�ӂ̈ʒu���Ԃ��Ă���B
		EditMode _em = IsMouseOnRectEdge(pt, objs[i], overlap);
        if (_em !=  EditMode::None )
        {
            objs[i].mOver = true;
            editMode = _em; // �I����Ԃɂ���
            _idx = i; // �ŏ��̈������Ԃ�
            break;
        }
        else
        {
            editMode = EditMode::None; // �I����Ԃ�����
        }
    }
    return _idx; // ��`���Ȃ��ꍇ��-1��Ԃ�
}

///////////////////////////////////////////////////////////////////////
//�^�C�g���o�[�ɉ摜�̃p�X��\��
void SetStringToTitlleBar(HWND hWnd, std::wstring _imgfolder, std::wstring _labelfolder, int _Idx, int _Total)
{
    std::wstring title =
        L"Annotation Tool - " + _imgfolder + L" - " + _labelfolder +
        L" [ " + std::to_wstring(_Idx) + L" / " + std::to_wstring(_Total) + L"]";
    SetWindowText(hWnd, title.c_str());
    return;
}

///////////////////////////////////////////////////////////////////////
// TaskDialogIndirect���g���ă_�C�A���O�{�b�N�X��\������֐�
// Yes/No�{�^���ƁA�u���񂩂�\�����Ȃ��v������
// �߂�l�́AIDYES, IDNO, IDCHECKBOX
//int ShowDialogWithCheckbox(HWND hwnd, const std::wstring& _message, const std::wstring& _title)
//{
//	// �_�C�A���O�̃{�^���̐ݒ�
//	TASKDIALOG_BUTTON buttons[] = {
//		{ IDYES, L"Yes" },
//		{ IDNO, L"No" }
//	};
//	// �_�C�A���O�̐ݒ�
//	TASKDIALOGCONFIG tdc = { sizeof(tdc) };
//	tdc.hwndParent = hwnd;
//	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS;
//	tdc.pszWindowTitle = _title.c_str();
//	tdc.pszContent = _message.c_str();
//	tdc.cButtons = ARRAYSIZE(buttons);
//	tdc.pButtons = buttons;
//	tdc.nDefaultButton = IDYES;
//	// �_�C�A���O��\��
//	int nButtonPressed = 0;
//	BOOL bCheckBoxChecked = FALSE;
//	TaskDialogIndirect(&tdc, &nButtonPressed, nullptr, &bCheckBoxChecked);
//	return nButtonPressed; // �߂�l�́AIDYES, IDNO, IDCHECKBOX
//}

// minx, miny �́u����ȉ��Ȃ� NG�v�Ƃ݂Ȃ��ŏ����E�ŏ�����
// #include "CPP_Anotation6.h"  // ImgObject/LabelObj ��` :contentReference[oaicite:0]{index=0}

// _startIdx: �����J�n�̉摜�C���f�b�N�X�i����� 0�j
// minW/minH: ���E������臒l�i���K���ς� or �s�N�Z���ɍ��킹�āj
std::optional<size_t> jumpImgWithIgnoreBox(
    const std::vector<ImgObject>& imgObjs,
    size_t _startIdx ,
    float minW ,
    float minH )
{
    if (_startIdx >= imgObjs.size())
        return std::nullopt;

    for (size_t imgIdx = _startIdx; imgIdx < imgObjs.size(); ++imgIdx)
    {
        const auto& img = imgObjs[imgIdx];
        for (const auto& lb : img.objs)
        {
            // �X�P�[�����` or �m�[�}����`��I��
            //const auto& rc = (lb.Rct_Scale.Width > 0 && lb.Rct_Scale.Height > 0)
            //    ? lb.Rct_Scale
            //    : lb.Rct;
            //const auto& rc = lb.Rct;
            //if (rc.Width <= minW || rc.Height <= minH)
            //    return imgIdx;
            if(isIgnoreBox(lb, minW, minH))
            {
                return imgIdx; // �ŏ��Ɍ��������摜�C���f�b�N�X��Ԃ�
			}
        }
    }
    return std::nullopt;
}

bool isIgnoreBox(
    const LabelObj& obj,
    float minW,
    float minH)
{    
	bool _ret = false;
    _ret = (obj.Rct.Width <= minW || obj.Rct.Height <= minH);
    return _ret;
}

/// @brief �t�H���_�p�X�̖����f�B���N�g�����̒��O��"deleted"�t�H���_��}������
/// @param folderPath ���̃t�H���_�p�X�i��: L"C:\\hoge1\\hoge2\\labels"�j
/// @return �}����̃t�H���_�p�X�i��: L"C:\\hoge1\\hoge2\\deleted\\labels"�j
std::wstring InsertSubFolder(const std::wstring& folderPath, const std::wstring& _sub)
{
    // ������'\'�܂���'/'������
    std::wstring path = folderPath;
    if (!path.empty() && (path.back() == L'\\' || path.back() == L'/')) {
        path.pop_back();
    }

    // �Ō�̋�؂蕶���ʒu���擾
    size_t pos = path.find_last_of(L"\\/");
    std::wstring parent, leaf;
    if (pos == std::wstring::npos) {
        // ��؂蕶���Ȃ��Ȃ�S�̂�leaf�Ƃ݂Ȃ�
        parent.clear();
        leaf = path;
    }
    else {
        parent = path.substr(0, pos);
        leaf = path.substr(pos + 1);
    }

    // �g�ݗ���
    if (!parent.empty()) {
        return parent + L"\\" + _sub + L"\\" + leaf;
        //return parent + L"\\deleted\\" + leaf;
    }
    else {
        return _sub + L"\\" + leaf;
        //return L"deleted\\" + leaf;
    }
}

///////////////////////////////////////////////////////////////////////////////////
inline void FlushLeftMouseClick()
{
    // 1) ���{�^�����������܂ő҂�
    while (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        Sleep(10);
    }

    // 2) �L���[���̎c���Ă��� WM_LBUTTONDOWN �` WM_LBUTTONUP ����菜��
    MSG msg;
    while (PeekMessage(&msg, nullptr,
        WM_LBUTTONDOWN, WM_LBUTTONUP,
        PM_REMOVE))
    {
        // �������Ȃ�
    }
}

///////////////////////////////////////////////////////////////////////////////////
// �w��idx�̉摜�ƃ��x�����ړ�
int MoveCurrentImageAndLabel(HWND hWnd, int imgIdx)
{
    if (imgIdx < 0 || imgIdx >= static_cast<int>(GP.imgObjs.size())) {
        MessageBox(hWnd, L"�����ȉ摜�C���f�b�N�X�ł��B", L"�G���[", MB_OK | MB_ICONERROR);
        return -1; // �G���[�R�[�h
    }
    // ���݂̉摜�ƃ��x����ۑ�
    SaveLabelsToFileSingle(hWnd, GP.imgIdx, 0.0f);

    std::wstring _ImgFilePath = GP.imgObjs[GP.imgIdx].path;
    std::wstring _fn1 = GetOnlyFileNameFormPath(_ImgFilePath);
    std::wstring _LabelFilePath = GP.labelFolderPath + L"\\" + _fn1 + L".txt";

    // �ړ���̃t�H���_�p�X���쐬
    std::wstring _tempImagePath = InsertSubFolder(GP.imgFolderPath, L"deleted");
    std::wstring _tempLabelPath = InsertSubFolder(GP.labelFolderPath, L"deleted");
    
    // �ړ���̃t�@�C���p�X���쐬
    std::wstring _tempImageFilePath = _tempImagePath + L"\\" + _fn1 + L".jpg";
    std::wstring _tempLabelFilePath = _tempLabelPath + L"\\" + _fn1 + L".txt";

	// �ړ���̃t�H���_�����݂��Ȃ��ꍇ�͍쐬
    if (!PathFileExistsW(_tempImagePath.c_str()))
    {
        //if (!CreateDirectoryW(_tempImagePath.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        if(!std::filesystem::create_directories(_tempImagePath))
        {
            MessageBox(hWnd, _tempImagePath.c_str(), L"�ړ���̃t�H���_�̍쐬�Ɏ��s���܂���", MB_OK | MB_ICONERROR);
            FlushLeftMouseClick();
            return -1; // �G���[�R�[�h
        }
	}
    if (!PathFileExistsW(_tempLabelPath.c_str())) {
        //if (!CreateDirectoryW(_tempLabelPath.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
        if (!std::filesystem::create_directories(_tempLabelPath))
        {
            MessageBox(hWnd, _tempLabelPath.c_str(), L"�ړ���̃t�H���_�̍쐬�Ɏ��s���܂���", MB_OK | MB_ICONERROR);
            FlushLeftMouseClick();
            return -1; // �G���[�R�[�h
        }
    }
    
    // ���������瑶�݃`�F�b�N��ǉ�
    auto FileExists = [](const std::wstring& path) -> bool {
        return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
        };

    if (!FileExists(_ImgFilePath)) {
        MessageBox(hWnd, (_ImgFilePath + L"\n(�摜��������܂���)").c_str(),
            L"�ړ����s", MB_OK | MB_ICONERROR);
        FlushLeftMouseClick();
        return -1;
    }
#ifdef _DEBUG
     if (!FileExists(_LabelFilePath)) {
        MessageBox(hWnd, (_LabelFilePath + L"\n(���x����������܂��� ���s���܂�)").c_str(),
			L"�x��", MB_OK | MB_ICONWARNING);
        FlushLeftMouseClick();
     }
#endif   

#ifndef RELEASE_IMAGE
    // �@ �摜�t�@�C���̃n���h����������Ă���ړ�����
    if (GP.imgIdx < GP.imgObjs.size() && GP.imgObjs[GP.imgIdx].image)
        GP.imgObjs[GP.imgIdx].image.reset();   // �� unique_ptr �����Z�b�g
#endif

    if (!MoveFileEx(_ImgFilePath.c_str(),
        _tempImageFilePath.c_str(),   // ���t�@�C�����t���p�X��n��
        MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING))
    {
        std::wstring _msg = _ImgFilePath + L"\n" + _tempImageFilePath;
        MessageBox(hWnd, _msg.c_str(), L"�摜�̈ړ��Ɏ��s���܂���", MB_OK | MB_ICONERROR);
        FlushLeftMouseClick();
        return -1;
    }

    // ���x��
    if (!MoveFileEx(_LabelFilePath.c_str(),
        _tempLabelFilePath.c_str(),   // ������������l
        MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING))
    {
        std::wstring _msg = _LabelFilePath + L"\n" + _tempLabelFilePath;
#ifdef _DEBUG
        MessageBox(hWnd, _msg.c_str(), L"���x���t�@�C���͂Ȃ������悤�ł� ���s���܂�",
            MB_OK | MB_ICONWARNING);
        FlushLeftMouseClick();
#endif   
    }

    //��������̉摜�ƃ��x�����폜
    GP.imgObjs.erase(GP.imgObjs.begin() + GP.imgIdx); // ���݂̉摜�ƃ��x�����폜
    //GP.imgIdx = imgIdx;

	//�}�E�X�̃N���b�N���t���b�V��
    //FlushLeftMouseClick();

    // �^�C�g���o�[�ɐV�����摜�ƃ��x���̃p�X��\��
    SetStringToTitlleBar(hWnd, GP.imgFolderPath, GP.labelFolderPath, GP.imgIdx, static_cast<int>(GP.imgObjs.size()));
    // �ĕ`��
    InvalidateRect(hWnd, NULL, TRUE);

    FlushLeftMouseClick();
    return 0; // ����
}

