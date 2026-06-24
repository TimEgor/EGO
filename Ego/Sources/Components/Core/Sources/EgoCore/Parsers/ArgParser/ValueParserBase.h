#pragma once

#include <cstring>
#include <string>

#include "ValueParserInterface.h"

namespace ego
{
    template <>
    struct ArgValueParser<char*>
    {
        static void parse(std::string_view _str, char*& _val)
        {
            if (_val == nullptr)
            {
                return;
            }

            const std::string value = text_value_parser_detail::ToString(_str);
            std::memcpy(_val, value.c_str(), value.size() + 1);
        }
    };

    template <>
    struct ArgValueParser<char**>
    {
        static void parse(std::string_view _str, char**& _val)
        {
            if (_val == nullptr || *_val == nullptr)
            {
                return;
            }

            const std::string value = text_value_parser_detail::ToString(_str);
            std::memcpy(*_val, value.c_str(), value.size() + 1);
        }
    };
} // namespace ego
