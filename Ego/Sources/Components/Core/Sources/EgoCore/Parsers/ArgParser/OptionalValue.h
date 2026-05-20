#pragma once

#include <functional>

#include "ValueParserBase.h"

namespace ego
{
    class BaseOptionValue
    {
    public:
        BaseOptionValue(const char* _argStr): m_argStr(_argStr) {}
        virtual ~BaseOptionValue() = default;

        virtual void parse(const char* _valStr) = 0;

    private:
        const char* m_argStr;
    };

    template <typename OptionValType>
    class OptionValue final : public BaseOptionValue
    {
    public:
        using SetterType = std::function<void(const char*, OptionValType&)>;

        OptionValue(const char* _argStr, OptionValType& _value, SetterType _setter = nullptr)
            : BaseOptionValue(_argStr),
              m_value(_value),
              m_setter(_setter) {}

        virtual void parse(const char* _valStr) override
        {
            if (m_setter)
            {
                m_setter(_valStr, m_value);
            }
            else
            {
                arg_parser_interface::ParseValue(_valStr, m_value);
            }
        }

    private:
        OptionValType& m_value;
        SetterType m_setter = nullptr;
    };
}
