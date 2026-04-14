#pragma once

#include <Windows.h>
#include <string>

inline std::wstring UTF8ToWstring(const char* str_)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, str_, -1, nullptr, 0);
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str_, -1, result.data(), size);

    if (!result.empty())
        result.pop_back();

    return result;
}