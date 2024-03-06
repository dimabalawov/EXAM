
#include "header.h"

namespace fs = std::filesystem;
using namespace std;
struct Word
{
	wstring word;
	int amount = 0;
};

INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
void OpenFile(HWND hwnd);
void AddWord(HWND hEdit,HWND hwnd);
void RefreshList(HWND hList);
void OpenFolder();
DWORD WINAPI MainThread(LPVOID lp);
DWORD WINAPI InfoThread(LPVOID lp);
DWORD WINAPI EncryptedThread(LPVOID lp);

HINSTANCE hInstance;
HANDLE hMutex,hMain,hInfo,hEncrypted;
HWND hList, hEdit, hWordsAmount, hProgress, hTerminate,hStart;
vector<wstring> wordsarr{};
static bool isStarted = 0;
static bool isEnded = 0;

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpszCmdLine, int nCmdShow)
{
	hInstance=hInst;
	return DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL,DlgProc);
}


INT_PTR CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return TRUE;
	case WM_INITDIALOG:
		CreateMutex(0, FALSE, TEXT("{B8A2C367-10FE-494d-A869-841B2AF972E0}"));
		SendMessage(hProgress, PBM_SETSTEP, 1, 0);
		hMain = CreateThread(NULL, 0, MainThread, 0, CREATE_SUSPENDED, NULL);
		hEncrypted = CreateThread(NULL, 0, EncryptedThread, 0, CREATE_SUSPENDED, NULL);
		hInfo = CreateThread(NULL, 0, InfoThread, 0, CREATE_SUSPENDED, NULL);
		hList = GetDlgItem(hwnd, IDC_LIST1);
		hEdit= GetDlgItem(hwnd, IDC_EDIT1);
		hWordsAmount = GetDlgItem(hwnd, IDC_WORDS);
		hProgress = GetDlgItem(hwnd, IDC_PROGRESS1);
		hTerminate = GetDlgItem(hwnd, IDC_TERMINATE);
		hStart = GetDlgItem(hwnd, IDC_START);
		SetWindowText(hWordsAmount, L"0");
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_FILES)
			OpenFile(hwnd);
		if (LOWORD(wParam) == IDC_ADDWORD)
			AddWord(hEdit,hwnd);
		if (LOWORD(wParam) == IDC_CLEAR)
		{
			wordsarr.clear();
			RefreshList(hList);
		}
		if (LOWORD(wParam) == IDC_START)
		{
			if (isEnded)
			{
				hMain = CreateThread(NULL, 0, MainThread, 0, 0, NULL);
				hEncrypted = CreateThread(NULL, 0, EncryptedThread, 0, 0, NULL);
				hInfo = CreateThread(NULL, 0, InfoThread, 0, 0, NULL);
				SetWindowText(hStart, L"Пауза");
				isEnded = 0;
			}
			else if (isStarted)
			{
				SuspendThread(hMain);
				SuspendThread(hEncrypted);
				SuspendThread(hInfo);
				SetWindowText(hStart, L"Старт");
			}
			else
			{
				ResumeThread(hMain);
				ResumeThread(hEncrypted);
				ResumeThread(hInfo);
				SetWindowText(hStart, L"Пауза");
			}
			isStarted = !isStarted;
		}
		if (LOWORD(wParam) == IDC_TERMINATE)
		{
			TerminateThread(hMain, 0);
			TerminateThread(hInfo, 0);
			TerminateThread(hEncrypted, 0);
			EnableWindow(hTerminate, 0);
			EnableWindow(hStart, 0);
			SetWindowText(hStart, L"Старт");
		}
		if (LOWORD(wParam) == IDC_DIRECTORY)
			OpenFolder();
		return TRUE;
	}
	return FALSE;
}
void RefreshList(HWND hList)
{
	SendMessage(hList, LB_RESETCONTENT, 0, 0);
	for (size_t i = 0; i < wordsarr.size(); i++)
		SendMessage(hList, LB_ADDSTRING, 0, LPARAM(wordsarr[i].c_str()));

}
void AddWord(HWND hEdit,HWND hwnd)
{
	int lenght = GetWindowTextLength(hEdit);
	if (lenght == 0)
	{
		MessageBox(hwnd, L"Введите слово", L"Ошибка", 0);
		return;
	}
	WCHAR* buff = new WCHAR[lenght + 1];
	GetWindowText(hEdit, buff,lenght+1);
	wstring temp(buff);
	delete[] buff;
	if (find(wordsarr.begin(), wordsarr.end(), temp) == wordsarr.end())
		wordsarr.push_back(temp);
	else
	{
		MessageBox(hwnd, L"Слово уже есть в списке", L"Ошибка", 0);
		return;
	}
	RefreshList(hList);
	SetWindowText(hEdit, L"");
}
void OpenFile(HWND hwnd) {
	OPENFILENAME ofn;
	WCHAR szFileName[MAX_PATH] = L"";

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = L"txt";
	if (GetOpenFileName(&ofn) == TRUE) {
		wifstream file(ofn.lpstrFile);
		wstring line;
		while (getline(file, line))
		{
			if (find(wordsarr.begin(), wordsarr.end(), line) == wordsarr.end()) 
				wordsarr.push_back(line);
		}
		file.close();
		RefreshList(hList);
	}
}
DWORD WINAPI MainThread(LPVOID lp)
{
	fs::path new_directory = "C:\\Users\\dimab\\Desktop\\EXAM\\Multithreaded application\\NewDirectory";
	if (!fs::exists(new_directory))
		fs::create_directory(new_directory);
	int amountofwords = 0;
	int amountoffiles = 0;
	const string directory_path = "C:\\Users\\dimab\\Desktop\\EXAM\\Multithreaded application";
	for (const auto& entry : fs::directory_iterator(directory_path))
	{
		if (entry.path().extension() == ".txt")
		{
			amountoffiles++;
		}
	}
	SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, (amountoffiles*2)));
	SendMessage(hProgress, PBM_SETPOS, 0, 0);
	for (const auto& entry : fs::directory_iterator(directory_path))
	{
		hMutex = OpenMutex(MUTEX_ALL_ACCESS, false, TEXT("{B8A2C367-10FE-494d-A869-841B2AF972E0}"));
		DWORD dwAnswer = WaitForSingleObject(hMutex, INFINITE);
		if (entry.path().extension() == ".txt") {
			std::wifstream file(entry.path());
			if (file.is_open()) {
				wstring line;
				bool isCopied = 0;
				while (getline(file, line))
				{
					for (const auto& word : wordsarr) {
						if (word == line) {
							amountofwords++;
							if (!isCopied)
							{
								fs::copy_file(entry.path(), new_directory / entry.path().filename(), fs::copy_options::overwrite_existing);
								isCopied = 1;
							}
						}
					}
				}
			}
			file.close();
			Sleep(500);
			SendMessage(hProgress, PBM_STEPIT, 0, 0);
		}
		ReleaseMutex(hMutex);
	}
	SetWindowText(hWordsAmount, to_wstring(amountofwords).c_str());
	SendMessage(hProgress, PBM_SETPOS, amountoffiles*2, 0);
	SetWindowText(hStart, L"Старт");
	isEnded = 1;
	isStarted = 0;
	return 0;
}
DWORD WINAPI InfoThread(LPVOID lp)
{
	vector<Word> words;
	wofstream infoFile(L"C:\\Users\\dimab\\Desktop\\EXAM\\Multithreaded application\\InfoFile.txt");
	const string directory_path = "C:\\Users\\dimab\\Desktop\\EXAM\\Multithreaded application";
	for (const auto& entry : fs::directory_iterator(directory_path))
	{
		hMutex = OpenMutex(MUTEX_ALL_ACCESS, false, TEXT("{B8A2C367-10FE-494d-A869-841B2AF972E0}"));
		DWORD dwAnswer = WaitForSingleObject(hMutex, INFINITE);
		if (entry.path().extension() == ".txt") {
			std::wifstream file(entry.path());
			if (file.is_open()) {
				wstring line;
				bool isCopied = 0;
				while (getline(file, line))
				{
					bool isFound = false;
					for (auto& word : words)
					{
						if (word.word == line)
						{
							word.amount++;
							isFound = true;
							break;
						}
					}
					if (!isFound)
					{
						Word newWord;
						newWord.word = line;
						newWord.amount = 1;
						words.push_back(newWord);
					}
					if (isCopied == 0)
					{
						infoFile << entry.path().filename();
						infoFile << "\t";
						infoFile << entry.path();
						infoFile << "\t";
						infoFile << "Bytes: ";
						infoFile << fs::file_size(entry.path());
						infoFile << "\n";
						isCopied = 1;
					}
				}
			}
			file.close();
		}
		ReleaseMutex(hMutex);
	}
	for (int i = 0; i < words.size(); i++)
	{
		for (int j = words.size() - 1; j > i; j--)
		{
			if (words[j - 1].amount < words[j].amount)
			{
				std::swap(words[j - 1], words[j]);
			}
		}
	}
	for (size_t i = 0; i < min(words.size(), size_t(10)); i++)
	{
		infoFile << words[i].word;
		infoFile << "\t\t";
		infoFile << words[i].amount;
		infoFile << "\n";
	}
	infoFile.close();
	return 0;
}
DWORD WINAPI EncryptedThread(LPVOID lp)
{
	wstring prefix = L"Encrypted_";
	fs::path new_directory = L"C:\\Users\\dimab\\Desktop\\EXAM\\Multithreaded application\\EncrytedDirectory";
	if (!fs::exists(new_directory))
		fs::create_directory(new_directory);
	const wstring directory_path = L"C:\\Users\\dimab\\Desktop\\EXAM\\Multithreaded application";
	for (const auto& entry : fs::directory_iterator(directory_path))
	{
		hMutex = OpenMutex(MUTEX_ALL_ACCESS, false, TEXT("{B8A2C367-10FE-494d-A869-841B2AF972E0}"));
		DWORD dwAnswer = WaitForSingleObject(hMutex, INFINITE);
		if (entry.path().extension() == L".txt" && entry.path().filename()!="InfoFile.txt")
		{
			std::wifstream file(entry.path());
			if (file.is_open()) {
				wstring line;
				wstring filename = new_directory / (prefix + entry.path().filename().wstring());
				wofstream encrypted(filename);
				encrypted << "";
				while (getline(file, line))
				{
					encrypted << L"*******";
					encrypted << L"\n";
				}
				encrypted.close();
			}
			file.close();
			ReleaseMutex(hMutex);
			Sleep(500);
		}
	}
	return 0;
}
void OpenFolder()
{
	const WCHAR folderPath[MAX_PATH] = L"C:\\Users\\dimab\\Desktop\\EXAM\\Multithreaded application\\";
	if (PathIsDirectory(folderPath)) {
		ShellExecute(NULL, L"open", folderPath, NULL, NULL, SW_SHOWNORMAL);
	}
}
