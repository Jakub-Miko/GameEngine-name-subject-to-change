#include "WindowsOSApi.h"
#include <Windows.h>
#include <shlwapi.h>
#include <filesystem>
#include <string>

bool WindowsOSApi::OpenFileInDefaultApp(const std::string& filepath) {
    std::string file_path = std::filesystem::absolute(std::filesystem::path(filepath)).generic_string().c_str();
    auto find = file_path.find('/');
    while (find != file_path.npos) {
        file_path = file_path.replace(find, 1, "\\");
        find = file_path.find('/', find);
    }
	TCHAR szBuf[1000];
	DWORD cbBufSize = sizeof(szBuf);
    std::string extension = std::filesystem::path(file_path).extension().generic_string();
	cbBufSize = sizeof(szBuf);
	HRESULT hr = AssocQueryString(ASSOCF_REMAPRUNDLL | ASSOCF_NOTRUNCATE, ASSOCSTR_EXECUTABLE, extension.c_str()
		, NULL, szBuf, &cbBufSize);
    if (FAILED(hr)) { 
        return false;
    
    }
	std::string exec(szBuf, cbBufSize-1);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));


    std::string command = std::string("\"" + exec + "\"  \"" + file_path + "\"");

    // start the program up
    CreateProcess(NULL,   // the path
        const_cast<char*>(command.c_str()),        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );
    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}