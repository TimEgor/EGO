#pragma once

#include <cstdint>
#include <string>

#include "ValueParserInterface.h"

namespace ego
{
    namespace arg_parser_interface
    {
        inline void ParseValue(const char* _str, int8_t& _val)
        {
            _val = std::stoi(_str);
        }

        inline void ParseValue(const char* _str, int16_t& _val)
        {
            _val = std::stoi(_str);
        }

        inline void ParseValue(const char* _str, int32_t& _val)
        {
            _val = std::stoi(_str);
        }

        inline void ParseValue(const char* _str, int64_t& _val)
        {
            _val = std::stoll(_str);
        }

        inline void ParseValue(const char* _str, uint8_t& _val)
        {
            _val = static_cast<uint8_t>(std::stoul(_str));
        }

        inline void ParseValue(const char* _str, uint16_t& _val)
        {
            _val = static_cast<uint16_t>(std::stoul(_str));
        }

        inline void ParseValue(const char* _str, uint32_t& _val)
        {
            _val = std::stoul(_str);
        }

        inline void ParseValue(const char* _str, uint64_t& _val)
        {
            _val = std::stoull(_str);
        }

        inline void ParseValue(const char* _str, float& _val)
        {
            _val = std::stof(_str);
        }

        inline void ParseValue(const char* _str, double& _val)
        {
            _val = std::stod(_str);
        }

        inline void ParseValue(const char* _str, char** _val)
        {
            std::strcpy(*_val, _str);
        }

        inline void ParseValue(const char* _str, std::string& _val)
        {
            _val = _str;
        }
    }
}
