#pragma once

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
			: m_xmlNodeIterator(_iter) {}

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
			: m_xmlNodeIterator(_iter) {}

		bool operator==(const XmlNamedNodeIterator& _iter) const;
		bool operator!=(const XmlNamedNodeIterator& _iter) const;

		XmlNode operator*() const;

		XmlNamedNodeIterator& operator++();
		XmlNamedNodeIterator operator++(int);

		XmlNamedNodeIterator& operator--();
		XmlNamedNodeIterator operator--(int);
	};

	template<typename Iter>
	class XmlNodeRange final
	{
	private:
		Iter m_begin;
		Iter m_end;

	public:
		XmlNodeRange(Iter _begin, Iter _end)
			: m_begin(_begin),
			  m_end(_end) {}

		Iter begin() const { return m_begin; }
		Iter end() const { return m_end; }

		bool empty() const { return m_begin == m_end; }
	};

	class XmlNode final
	{
	private:
		pugi::xml_node m_xmlNode;

	public:
		XmlNode() = default;
		XmlNode(pugi::xml_node _pugiXmlNode)
			: m_xmlNode(_pugiXmlNode) {}

		operator bool() const { return m_xmlNode; }

		XmlNode getChild(const char* _name) const;

		XmlNodeRange<XmlNodeIterator> getAllChildren() const;
		XmlNodeRange<XmlNamedNodeIterator> getChildren(const char* _name) const;

		XmlNode getFirstChild() const;
		XmlNode getLastChild() const;

		XmlNode getNextSibling() const;
		XmlNode getPrevSibling() const;

		const char* getName() const;
		bool setName(const char* _name);
		
		XmlNodeValue getValue() const;
	};
}
