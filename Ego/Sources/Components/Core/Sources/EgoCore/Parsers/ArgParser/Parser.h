#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>

#include "OptionalValue.h"

namespace ego
{
    class ArgParser final
    {
        using OptionContainer = std::unordered_map<std::string, std::unique_ptr<BaseOptionValue>>;

    private:
        OptionContainer m_options;

    public:
        ArgParser() = default;
        ~ArgParser() = default;

        ArgParser(const ArgParser&) = delete;
        ArgParser& operator=(const ArgParser&) = delete;

        ArgParser(ArgParser&&) noexcept = default;
        ArgParser& operator=(ArgParser&&) noexcept = default;

        template <typename OptionT>
        void addOptionValue(const char* _optionName, OptionT& _value)
        {
            setOptionValue(_optionName, std::make_unique<OptionValue<OptionT>>(_value));
        }

        template <typename OptionT>
        void addOptionValue(const char* _optionName, OptionT& _value, typename OptionValue<OptionT>::SetterType _setter)
        {
            setOptionValue(_optionName, std::make_unique<OptionValue<OptionT>>(_value, _setter));
        }

        template <typename OptionT>
        void addOptionValue(const char* _optionName, OptionT& _value, std::nullptr_t)
        {
            addOptionValue(_optionName, _value);
        }

        template <typename OptionT>
        void addOptionValue(const char* _optionName, OptionT& _value, typename OptionValue<OptionT>::LegacySetterType _setter)
        {
            setOptionValue(_optionName, std::make_unique<OptionValue<OptionT>>(_value, _setter));
        }

        void parse(int _argc, char** _argv);
        void clear();

    private:
        void setOptionValue(const char* _optionName, std::unique_ptr<BaseOptionValue> _optionValue);
    };
} // namespace ego
