#pragma once

namespace ego
{
    namespace arg_parser_interface
    {
        template <typename T>
        void ParseValue(const char* _str, T& _val)
        {
            static_assert(false && "Invalid type for Args parser");
        }

        template <typename T>
        void ParseValue(const char* _str, T* _val)
        {
            static_assert(false && "Invalid type for Args parser");
        }
    }
}
