#pragma once

#include <string>
#include <string_view>

#include "NodeValueParserBase.h"

#include <pugixml.hpp>

namespace ego
{
    class XmlNodeValue final
    {
    private:
        pugi::xml_text m_xmlText;

    public:
        XmlNodeValue() = default;
        XmlNodeValue(pugi::xml_text _pugiXmlText)
            : m_xmlText(_pugiXmlText)
        {
        }

        template <typename T>
        T get() const;

        const char* getRaw() const
        {
            return m_xmlText.get();
        }
        std::string_view getRawView() const;

        template <typename T>
        bool tryGet(T& _val) const;

        template <typename T>
        T getOr(const T& _defaultVal) const;

        void set(std::string_view _val);
        void set(const std::string& _val);

        template <typename T>
        void set(const T& _val);
    };

    template <typename T>
    T XmlNodeValue::get() const
    {
        T result{};
        ParseXmlValue(getRawView(), result);
        return result;
    }

    inline std::string_view XmlNodeValue::getRawView() const
    {
        const char* rawValue = getRaw();
        return rawValue != nullptr ? std::string_view(rawValue) : std::string_view();
    }

    template <typename T>
    bool XmlNodeValue::tryGet(T& _val) const
    {
        return TryParseXmlValue(getRawView(), _val);
    }

    template <typename T>
    T XmlNodeValue::getOr(const T& _defaultVal) const
    {
        T result = _defaultVal;
        return tryGet(result) ? result : _defaultVal;
    }

    inline void XmlNodeValue::set(std::string_view _val)
    {
        const std::string value = _val.empty() ? std::string() : std::string(_val.data(), _val.size());
        m_xmlText.set(value.c_str());
    }

    inline void XmlNodeValue::set(const std::string& _val)
    {
        m_xmlText.set(_val.c_str());
    }

    template <typename T>
    void XmlNodeValue::set(const T& _val)
    {
        m_xmlText.set(_val);
    }
} // namespace ego
