#pragma once

#include <format>
#include <string>
#include <string_view>

namespace ego
{
    template <class... Args>
    std::string StringFormat(std::string_view _fmt, Args&&... _args)
    {
        return std::vformat(_fmt, std::make_format_args(_args...));
    }
} // namespace ego