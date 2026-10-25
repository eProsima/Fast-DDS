/**
 * @file Win32Dll.cpp
 * @brief Deletes all residual files left over after crashes when loading and unloading the DLL.
 */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#ifdef _MSC_VER
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#else
#include <strings.h>
#endif

static bool win32_test_file_open(const std::string& fileName_in)
{
    HANDLE hFile = CreateFileA(fileName_in.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return true; // File is open
    else
    {
        // File not open
        CloseHandle(hFile);
        return false;
    }
}

static bool win32_test_dir_exists(const std::string& dirName_in)
{
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;  // Something is wrong with the path!
    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;   // This is a directory!
    return false;      // This is not a directory!
}

static bool win32_test_file_exists(const std::string& fileName_in)
{
    DWORD attributes = GetFileAttributesA(fileName_in.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

static bool str_endsWith(std::string& text, std::string test)
{
    if (test.empty() || text.empty()) return false;
    if (test.length() > text.length()) return false;
    size_t offset = text.length() - test.length();
    return strncasecmp(text.c_str() + offset, test.c_str(), test.length()) == 0;
}

static void cleanup_eprosima()
{
    std::string interprocessdata("C:\\ProgramData\\eprosima\\fastrtps_interprocess");
    if (!win32_test_dir_exists(interprocessdata)) return;
    std::string search = interprocessdata + "\\*";
    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA(search.c_str(), &data);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do
    {
        if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            std::string filename(data.cFileName);
            if (str_endsWith(filename, "_el") || str_endsWith(filename, "_mutex")) continue;
            std::string file = interprocessdata + "\\" + filename;
            if (win32_test_file_open(file)) continue;
            std::string path = file + "_el";
            if (win32_test_file_exists(path)) DeleteFileA(path.c_str());
            path = file + "_mutex";
            if (win32_test_file_exists(path)) DeleteFileA(path.c_str());
            DeleteFileA(file.c_str());
        }
    } while (FindNextFileA(hFind, &data));
    FindClose(hFind);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_ATTACH:
    case DLL_PROCESS_DETACH:
        cleanup_eprosima();
        break;
    }
    return TRUE;
}
#endif
