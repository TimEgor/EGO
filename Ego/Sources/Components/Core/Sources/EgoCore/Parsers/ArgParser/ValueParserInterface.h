#pragma once

#include <string_view>

#include "EgoCore/Parsers/TextValueParser/TextValueParser.h"

namespace ego
{
    template <typename T>
    struct ArgValueParser
    {
        static void parse(std::string_view _str, T& _val)
        {
            ParseTextValue(_str, _val);
        }
    };

    template <typename T>
    void ParseArgValue(std::string_view _str, T& _val)
    {
        ArgValueParser<T>::parse(_str, _val);
    }

    namespace arg_parser_interface
    {
        template <typename T>
        void ParseValue(std::string_view _str, T& _val)
        {
            ParseArgValue(_str, _val);
        }

        template <typename T>
        void ParseValue(const char* _str, T& _val)
        {
            ParseValue(_str != nullptr ? std::string_view(_str) : std::string_view(), _val);
        }
    }
}
