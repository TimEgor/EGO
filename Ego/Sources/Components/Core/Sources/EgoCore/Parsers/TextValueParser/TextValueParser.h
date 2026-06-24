#pragma once

#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ego
{
    template <typename>
    inline constexpr bool TextValueParserAlwaysFalse = false;

    template <typename T>
    struct TextValueParser
    {
        static void parse(std::string_view _str, T& _val)
        {
            static_assert(TextValueParserAlwaysFalse<T>, "Invalid type for text parser");
        }
    };

    template <typename T>
    void ParseTextValue(std::string_view _str, T& _val)
    {
        TextValueParser<T>::parse(_str, _val);
    }

    template <typename T>
    bool TryParseTextValue(std::string_view _str, T& _val)
    {
        try
        {
            ParseTextValue(_str, _val);
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    namespace text_value_parser_detail
    {
        inline std::string ToString(std::string_view _str)
        {
            return _str.empty() ? std::string() : std::string(_str.data(), _str.size());
        }
    } // namespace text_value_parser_detail

    template <>
    struct TextValueParser<int8_t>
    {
        static void parse(std::string_view _str, int8_t& _val)
        {
            _val = static_cast<int8_t>(std::stoi(text_value_parser_detail::ToString(_str)));
        }
    };

    template <>
    struct TextValueParser<int16_t>
    {
        static void parse(std::string_view _str, int16_t& _val)
        {
            _val = static_cast<int16_t>(std::stoi(text_value_parser_detail::ToString(_str)));
        }
    };

    template <>
    struct TextValueParser<int32_t>
    {
        static void parse(std::string_view _str, int32_t& _val)
        {
            _val = static_cast<int32_t>(std::stoi(text_value_parser_detail::ToString(_str)));
        }
    };

    template <>
    struct TextValueParser<int64_t>
    {
        static void parse(std::string_view _str, int64_t& _val)
        {
            _val = std::stoll(text_value_parser_detail::ToString(_str));
        }
    };

    template <>
    struct TextValueParser<uint8_t>
    {
        static void parse(std::string_view _str, uint8_t& _val)
        {
            _val = static_cast<uint8_t>(std::stoul(text_value_parser_detail::ToString(_str)));
        }
    };

    template <>
    struct TextValueParser<uint16_t>
    {
        static void parse(std::string_view _str, uint16_t& _val)
        {
            _val = static_cast<uint16_t>(std::stoul(text_value_parser_detail::ToString(_str)));
        }
    };

    template <>
    struct TextValueParser<uint32_t>
    {
        static void parse(std::string_view _str, uint32_t& _val)
        {
            _val = static_cast<uint32_t>(std::stoul(text_value_parser_detail::ToString(_str)));
        }
    };

    template <>
    struct TextValueParser<uint64_t>
    {
        static void parse(std::string_view _str, uint64_t& _val)
        {
            _val = std::stoull(text_value_parser_detail::ToString(_str));
        }
    };

    template <>
    struct TextValueParser<float>
    {
        static void parse(std::string_view _str, float& _val)
        {
            _val = std::stof(text_value_parser_detail::ToString(_str));
        }
    };

    template <>
    struct TextValueParser<double>
    {
        static void parse(std::string_view _str, double& _val)
        {
            _val = std::stod(text_value_parser_detail::ToString(_str));
        }
    };

    template <>
    struct TextValueParser<bool>
    {
        static void parse(std::string_view _str, bool& _val)
        {
            if (_str == "true" || _str == "1")
            {
                _val = true;
                return;
            }

            if (_str == "false" || _str == "0")
            {
                _val = false;
                return;
            }

            throw std::invalid_argument("Invalid bool text value");
        }
    };

    template <>
    struct TextValueParser<std::string>
    {
        static void parse(std::string_view _str, std::string& _val)
        {
            if (_str.empty())
            {
                _val.clear();
                return;
            }

            _val.assign(_str.data(), _str.size());
        }
    };

    template <>
    struct TextValueParser<std::string_view>
    {
        static void parse(std::string_view _str, std::string_view& _val)
        {
            _val = _str;
        }
    };
} // namespace ego
