#pragma once

#include <string>

namespace ego
{
    std::wstring ConvertStringToWString(const std::string& _str);
    std::string ConvertWStringToString(const std::wstring& _wstr);

    template <typename T>
    std::string ConvertToString(T _val)
    {
        return std::to_string(_val);
    }
} // namespace ego
