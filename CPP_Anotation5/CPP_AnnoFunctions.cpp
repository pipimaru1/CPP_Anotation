#include "pch.h"
#include "framework.h"
#include "CPP_AnnoGblParams.h"
#include "CPP_AnnoFunctions.h"
#include "CPP_Anotation5.h"


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

                //_imgObjs.push_back(_imgobj);
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
    if (N == 0) return 0;

    // 2) ���ʃR���e�i�����炩���ߊm�ۂ��Ă���
    _imgObjs.resize(N);

    // 3) �摜���[�h�����
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < N; ++i) {
        ImgObject& img = _imgObjs[i];
        img.path = fileList[i];

        // unique_ptr �ŊǗ�
        auto image = std::make_unique<Gdiplus::Image>(img.path.c_str());
        if (image->GetLastStatus() != Gdiplus::Ok) {
            // ���[�h���s���͑�փC���[�W
            image = std::make_unique<Gdiplus::Image>(L"NO Image");
        }
        img.image = std::move(image);
    }

    return N;
}

///////////////////////////////////////////////////////////////////////
// ��`�̍��W�𐳋K������֐�
void NormalizeRect(RectF& r) 
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
// LabelObj���t�@�C���ۑ����邽�߂̕����񐶐��֐�
// ���͒l��LabelObj
// �o�͒l�͕����� std::wstring
// UTF-8�ŕۑ�����
// YOLO�`���ŕۑ�����
// wchar_t �� UTF-8 �̕ϊ��iC++17 �̏ꍇ�j
std::string LabelsToString(
    const LabelObj& obj, 
	int mode = 0 // 0:default, 1:yolo
)
{
    std::ostringstream oss;
    // ���l�ƃX�y�[�X�����Ȃ̂ŁA���ʂ� narrow string �� OK
    if (mode == 0)
    {
        oss << obj.ClassNum << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.X) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Y) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Width) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Height);
    }
	else if (mode == 1)
	{
        oss << obj.ClassNum << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.X + obj.rect.Width /2) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Y + obj.rect.Height/2) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Width) << ' '
            << std::fixed << std::setprecision(6) << static_cast<double>(obj.rect.Height);
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
	int mode = 0 // 0:default, 1:yolo
)
{
	// UTF-8�ŕۑ����邽�߂̐ݒ�
	std::ofstream file(fileName, std::ios::binary);

	// �t�@�C���I�[�v��
	if (!file.is_open()) {
		return false; // �t�@�C���I�[�v�����s
	}
	for (const auto& obj : objs) {
		file << LabelsToString(obj, mode) << std::endl;
	}
	file.close();
	return true;
}

///////////////////////////////////////////////////////////////////////
// �t�@�C���ۑ����邽�߂̃t�@�C�������_�C�A���O�{�b�N�X�Ŏ擾����֐�
// ���͒l�̓E�B���h�E�n���h��
// �o�͒l�̓t�@�C���� std::wstring
std::wstring GetFileName(HWND hWnd)
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
std::wstring GetFileNameFromPath(const std::wstring& filePath)
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
//�t�H���_�p�X�ƃt�@�C�����Ɗg���q���w�肵��
//�t�@�C�������݂���΁A���̃t�@�C���̃t���p�X��Ԃ��֐�
//�t�@�C�������݂��Ȃ���΁A�󕶎�""��Ԃ�
std::wstring GetFileNameFromPath(
    const std::wstring& folderpath, 
    const std::wstring& filename,
	const std::wstring& ext) // ".txt"�ȂǁB"."���܂ނ���
{
	std::wstring _fn = GetFileNameFromPath(filename);

	std::wstring fullPath = folderpath + L"\\" + _fn + ext;
	if (PathFileExistsW(fullPath.c_str())) 
    {
		return fullPath; // �t�@�C�������݂���ꍇ�A�t���p�X��Ԃ�
	}
	return L""; // �t�@�C�������݂��Ȃ��ꍇ�A�󕶎���Ԃ�
}



///////////////////////////////////////////////////////////////////////
// �֐� LoadAnno_Object(
// ImgObject& imgObj, //�f�[�^���i�[����ImgObject�N���X�̎Q��
// const std::wstring& folderpath, //�A�m�e�[�V�����t�@�C���̂���t�H���_�p�X
// const std::wstring& ext //�A�m�e�[�V�����t�@�C���̊g���q
// )
int LoadAnno_Object(
ImgObject& imgObj, //�f�[�^���i�[����ImgObject�N���X�̎Q��
const std::wstring& folderpath, //�A�m�e�[�V�����t�@�C���̂���t�H���_�p�X
const std::wstring& ext, //�A�m�e�[�V�����t�@�C���̊g���q
int mode //0:default, 1:yolo
)
{
	//�A�m�e�[�V�����t�@�C���̃t���p�X���擾
	std::wstring _fileName = GetFileNameFromPath(folderpath, imgObj.path, ext);
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
        while (file >> obj.ClassNum >> obj.rect.X >> obj.rect.Y >> obj.rect.Width >> obj.rect.Height)
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
            obj.rect.X = tmp_x - tmp_w / 2;
			obj.rect.Y = tmp_y - tmp_h / 2;
			obj.rect.Width = tmp_w;
            obj.rect.Height = tmp_h;
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
)
{
	int loadCount = 0; // �ǂݍ��񂾃A�m�e�[�V�����̐�
	for (int i = 0; i < imgObjs.size(); i++)
	{
        //�A�m�e�[�V�����t�@�C���̃t���p�X���擾
        std::wstring _fileName = GetFileNameFromPath(folderpath, imgObjs[i].path, ext);
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
            while (file >> obj.ClassNum >> obj.rect.X >> obj.rect.Y >> obj.rect.Width >> obj.rect.Height)
            {
				NormalizeRect(obj.rect); // ��`�̍��W�𐳋K��

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
				obj.rect.X = tmp_x - tmp_w / 2;
				obj.rect.Y = tmp_y - tmp_h / 2;
				obj.rect.Width = tmp_w;
				obj.rect.Height = tmp_h;

                NormalizeRect(obj.rect); // ��`�̍��W�𐳋K��
                
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


#include <fstream>
#include <vector>
#include <string>
#include <omp.h>    // �� �ǉ�

int LoadLabelFilesMP(
    std::vector<ImgObject>& imgObjs,
    const std::wstring& folderpath,
    const std::wstring& ext,
    int mode
)
{
    int loadCount = 0;
    const int N = static_cast<int>(imgObjs.size());

    // OpenMP ���񉻁F�e i �͓Ɨ������Ȃ̂ŋ����Ȃ�
#pragma omp parallel for schedule(dynamic) reduction(+:loadCount)
    for (int i = 0; i < N; ++i)
    {
        // �t�@�C�����擾�i�X���b�h���ƂɃ��[�J���ȕϐ��j
        std::wstring _fileName = GetFileNameFromPath(folderpath, imgObjs[i].path, ext);
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
                >> obj.rect.X >> obj.rect.Y
                >> obj.rect.Width >> obj.rect.Height)
            {
                NormalizeRect(obj.rect);

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
                obj.rect.X = tmp_x - tmp_w / 2;
                obj.rect.Y = tmp_y - tmp_h / 2;
                obj.rect.Width = tmp_w;
                obj.rect.Height = tmp_h;
                NormalizeRect(obj.rect);

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
)
{
	bool Left = false;
	bool Right = false;
	bool Top = false;
	bool Bottom = false;

    // ��`�̍��W
    float x0 = obj.rect.X * GP.width;
    float y0 = obj.rect.Y * GP.height;
    float w = obj.rect.Width * GP.width;
    float h = obj.rect.Height * GP.height;

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
    

    // ��`�̒��ɃJ�[�\��������ꍇ
	//if (pt.x >= x0 && pt.x <= x0 + w &&
	//	pt.y >= y0 && pt.y <= y0 + h)
	//	return 9; // ��

    // ��`�̊O�ɃJ�[�\��������ꍇ 

	return EditMode::None; // �O 
}
///////////////////////////////////////////////////////////////////////
//��`�̐���Ƀ}�E�X�J�[�\�������邩�ǂ����𔻒肷��֐�
int IsMouseOnRectEdge_old(
    const POINT& pt,
    const LabelObj& obj,
    int overlap
)
{
    // ��`�̍��W
    float x0 = obj.rect.X * GP.width;
    float y0 = obj.rect.Y * GP.height;
    float w = obj.rect.Width * GP.width;
    float h = obj.rect.Height * GP.height;

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

    // ��`�̒��ɃJ�[�\��������ꍇ
    //if (pt.x >= x0 && pt.x <= x0 + w &&
    //    pt.y >= y0 && pt.y <= y0 + h)
    //    return 5; // ��

    // ��`�̊O�ɃJ�[�\��������ꍇ 
    return 0; // �O
}

///////////////////////////////////////////////////////////////////////
//�}�E�X�J�[�\���Əd�Ȃ��`�̃C���f�b�N�X���擾����֐�
//�ŏ��̈������Ԃ�
//�d�Ȃ�����`�̃I�u�W�F�N�g�ɂ�_mOver��1�`8�̒l������
int GetIdxMouseOnRectEdge(
    const POINT& pt,
    std::vector<LabelObj>& objs,
	EditMode& editMode,
    int overlap
){
    int _idx=-1;

    //�S��
    for (int i = 0; i < objs.size(); i++)
        objs[i].mOver = false;

    for (int i = 0; i < objs.size(); i++)
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

