#pragma once

#include <string>
#include <unordered_map>

#include "OptionalValue.h"

namespace ego
{
    class ArgParser final
    {
        using OptionContainer = std::unordered_map<std::string, BaseOptionValue*>;

    private:
        OptionContainer m_options;

    public:
        ArgParser() = default;
        ~ArgParser();

        template <typename OptionT>
        void addOptionValue(
            const char* _optionName,
            OptionT& _value,
            typename OptionValue<OptionT>::SetterType _setter = nullptr
        )
        {
            m_options.insert(std::make_pair(_optionName, new OptionValue<OptionT>(_optionName, _value, _setter)));
        }

        void parse(int _argc, char** _argv);
        void clear();
    };
}
