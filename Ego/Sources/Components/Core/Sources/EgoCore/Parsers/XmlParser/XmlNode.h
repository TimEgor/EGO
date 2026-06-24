#pragma once

#include <string_view>

#include "XmlNodeValue.h"

namespace ego
{
    class XmlNode;

    class XmlNodeIterator final
    {
    private:
        pugi::xml_node_iterator m_xmlNodeIterator;

    public:
        XmlNodeIterator(pugi::xml_node_iterator _iter)
            : m_xmlNodeIterator(_iter)
        {
        }

        bool operator==(const XmlNodeIterator& _iter) const;
        bool operator!=(const XmlNodeIterator& _iter) const;

        XmlNode operator*() const;

        XmlNodeIterator& operator++();
        XmlNodeIterator operator++(int);

        XmlNodeIterator& operator--();
        XmlNodeIterator operator--(int);
    };

    class XmlNamedNodeIterator final
    {
    private:
        pugi::xml_named_node_iterator m_xmlNodeIterator;

    public:
        XmlNamedNodeIterator(pugi::xml_named_node_iterator _iter)
            : m_xmlNodeIterator(_iter)
        {
        }

        bool operator==(const XmlNamedNodeIterator& _iter) const;
        bool operator!=(const XmlNamedNodeIterator& _iter) const;

        XmlNode operator*() const;

        XmlNamedNodeIterator& operator++();
        XmlNamedNodeIterator operator++(int);

        XmlNamedNodeIterator& operator--();
        XmlNamedNodeIterator operator--(int);
    };

    template <typename Iter>
    class XmlNodeRange final
    {
    private:
        Iter m_begin;
        Iter m_end;

    public:
        XmlNodeRange(Iter _begin, Iter _end)
            : m_begin(_begin),
              m_end(_end)
        {
        }

        Iter begin() const
        {
            return m_begin;
        }
        Iter end() const
        {
            return m_end;
        }

        bool empty() const
        {
            return m_begin == m_end;
        }
    };

    class XmlNode final
    {
    private:
        pugi::xml_node m_xmlNode;

    public:
        XmlNode() = default;
        XmlNode(pugi::xml_node _pugiXmlNode)
            : m_xmlNode(_pugiXmlNode)
        {
        }

        operator bool() const
        {
            return m_xmlNode;
        }
        bool isValid() const
        {
            return m_xmlNode;
        }

        XmlNode getChild(const char* _name) const;

        XmlNodeRange<XmlNodeIterator> getAllChildren() const;
        XmlNodeRange<XmlNamedNodeIterator> getChildren(const char* _name) const;

        XmlNode getFirstChild() const;
        XmlNode getLastChild() const;

        XmlNode getNextSibling() const;
        XmlNode getPrevSibling() const;

        const char* getName() const;
        std::string_view getNameView() const;
        bool setName(const char* _name);

        bool hasAttribute(const char* _name) const;
        const char* getAttributeValue(const char* _name) const;

        template <typename T>
        bool tryGetAttribute(const char* _name, T& _val) const;

        template <typename T>
        T getAttributeOr(const char* _name, const T& _defaultVal) const;

        XmlNodeValue getValue() const;

        template <typename T>
        bool tryGetChildValue(const char* _name, T& _val) const;

        template <typename T>
        T getChildValueOr(const char* _name, const T& _defaultVal) const;
    };

    template <typename T>
    bool XmlNode::tryGetAttribute(const char* _name, T& _val) const
    {
        if (_name == nullptr)
        {
            return false;
        }

        const pugi::xml_attribute attribute = m_xmlNode.attribute(_name);
        return attribute && TryParseXmlValue(attribute.value(), _val);
    }

    template <typename T>
    T XmlNode::getAttributeOr(const char* _name, const T& _defaultVal) const
    {
        T result = _defaultVal;
        return tryGetAttribute(_name, result) ? result : _defaultVal;
    }

    template <typename T>
    bool XmlNode::tryGetChildValue(const char* _name, T& _val) const
    {
        if (_name == nullptr)
        {
            return false;
        }

        const XmlNode child = getChild(_name);
        return child && child.getValue().tryGet(_val);
    }

    template <typename T>
    T XmlNode::getChildValueOr(const char* _name, const T& _defaultVal) const
    {
        T result = _defaultVal;
        return tryGetChildValue(_name, result) ? result : _defaultVal;
    }
} // namespace ego

namespace ego::xml_parser_interface
{
    template <typename T>
    void GetValue(const XmlNode& _node, T& _val)
    {
        _val = _node.getValue().get<T>();
    }

    template <typename T>
    void GetValue(const XmlNode& _node, T* _val)
    {
        if (_val != nullptr)
        {
            GetValue(_node, *_val);
        }
    }

    template <typename T>
    void SetValue(XmlNode& _node, const T& _val)
    {
        _node.getValue().set(_val);
    }

    template <typename T>
    void SetValue(XmlNode& _node, const T* _val)
    {
        if (_val != nullptr)
        {
            SetValue(_node, *_val);
        }
    }
} // namespace ego::xml_parser_interface
