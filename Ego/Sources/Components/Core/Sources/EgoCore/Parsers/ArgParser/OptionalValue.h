#pragma once

#include <functional>
#include <string>
#include <string_view>

#include "ValueParserBase.h"

namespace ego
{
    class BaseOptionValue
    {
    public:
        virtual ~BaseOptionValue() = default;

        virtual void parse(std::string_view _valStr) = 0;
    };

    template <typename OptionValType>
    class OptionValue final : public BaseOptionValue
    {
    public:
        using SetterType = std::function<void(std::string_view, OptionValType&)>;
        using LegacySetterType = std::function<void(const char*, OptionValType&)>;

        OptionValue(OptionValType& _value)
            : m_value(_value)
        {
        }

        OptionValue(OptionValType& _value, SetterType _setter)
            : m_value(_value),
              m_setter(_setter)
        {
        }

        OptionValue(OptionValType& _value, LegacySetterType _setter)
            : m_value(_value),
              m_legacySetter(_setter)
        {
        }

        void parse(std::string_view _valStr) override
        {
            if (m_setter)
            {
                m_setter(_valStr, m_value);
                return;
            }

            if (m_legacySetter)
            {
                const std::string value(_valStr.data(), _valStr.size());
                m_legacySetter(value.c_str(), m_value);
                return;
            }

            ParseArgValue(_valStr, m_value);
        }

    private:
        OptionValType& m_value;
        SetterType m_setter;
        LegacySetterType m_legacySetter;
    };
} // namespace ego
