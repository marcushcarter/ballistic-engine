#pragma once
#include <string>
#include <vector>

namespace lumen::drivers {

struct Win32Dialogs
{
    static std::wstring save_file(const wchar_t* p_filter, const wchar_t* p_default_ext);
    static std::wstring open_file(const wchar_t* p_filter);
    static std::vector<std::wstring> open_files(const wchar_t* p_filter);

    static std::wstring open_folder(const wchar_t* p_title);
};

}