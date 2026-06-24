#include "Parser.h"

namespace
{
    struct ParsedArg
    {
        std::string_view m_name;
        std::string_view m_value;
        bool m_isValid = false;
    };

    ParsedArg ParseArg(std::string_view _arg)
    {
        ParsedArg result;

        const std::size_t valSeparator = _arg.find('=');
        if (valSeparator == std::string_view::npos)
        {
            return result;
        }

        result.m_name = _arg.substr(0, valSeparator);
        result.m_value = _arg.substr(valSeparator + 1);
        result.m_isValid = true;

        return result;
    }
} // namespace

void ego::ArgParser::parse(int _argc, char** _argv)
{
    if (_argc <= 0 || _argv == nullptr)
    {
        return;
    }

    for (int i = 0; i < _argc; ++i)
    {
        if (_argv[i] == nullptr)
        {
            continue;
        }

        const ParsedArg arg = ParseArg(_argv[i]);
        if (!arg.m_isValid)
        {
            continue;
        }

        auto findIter = m_options.find(std::string(arg.m_name.data(), arg.m_name.size()));
        if (findIter != m_options.end())
        {
            findIter->second->parse(arg.m_value);
        }
    }
}

void ego::ArgParser::clear()
{
    m_options.clear();
}

void ego::ArgParser::setOptionValue(const char* _optionName, std::unique_ptr<BaseOptionValue> _optionValue)
{
    if (_optionName == nullptr || !_optionValue)
    {
        return;
    }

    m_options[std::string(_optionName)] = std::move(_optionValue);
}
