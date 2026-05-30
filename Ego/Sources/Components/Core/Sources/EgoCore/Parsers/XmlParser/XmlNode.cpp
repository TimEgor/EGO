#include "XmlNode.h"

bool ego::XmlNodeIterator::operator==(const XmlNodeIterator& _iter) const
{
	return m_xmlNodeIterator == _iter.m_xmlNodeIterator;
}

bool ego::XmlNodeIterator::operator!=(const XmlNodeIterator& _iter) const
{
	return m_xmlNodeIterator != _iter.m_xmlNodeIterator;
}

ego::XmlNode ego::XmlNodeIterator::operator*() const
{
	return *m_xmlNodeIterator;
}

ego::XmlNodeIterator& ego::XmlNodeIterator::operator++()
{
	++m_xmlNodeIterator;
	return *this;
}

ego::XmlNodeIterator ego::XmlNodeIterator::operator++(int)
{
	const XmlNodeIterator tmp = *this;
	++m_xmlNodeIterator;
	return tmp;
}

ego::XmlNodeIterator& ego::XmlNodeIterator::operator--()
{
	--m_xmlNodeIterator;
	return *this;
}

ego::XmlNodeIterator ego::XmlNodeIterator::operator--(int)
{
	const XmlNodeIterator tmp = *this;
	--m_xmlNodeIterator;
	return tmp;
}

bool ego::XmlNamedNodeIterator::operator==(const XmlNamedNodeIterator& _iter) const
{
	return m_xmlNodeIterator == _iter.m_xmlNodeIterator;
}

bool ego::XmlNamedNodeIterator::operator!=(const XmlNamedNodeIterator& _iter) const
{
	return m_xmlNodeIterator != _iter.m_xmlNodeIterator;
}

ego::XmlNode ego::XmlNamedNodeIterator::operator*() const
{
	return *m_xmlNodeIterator;
}

ego::XmlNamedNodeIterator& ego::XmlNamedNodeIterator::operator++()
{
	++m_xmlNodeIterator;
	return *this;
}

ego::XmlNamedNodeIterator ego::XmlNamedNodeIterator::operator++(int)
{
	const XmlNamedNodeIterator tmp = *this;
	++m_xmlNodeIterator;
	return tmp;
}

ego::XmlNamedNodeIterator& ego::XmlNamedNodeIterator::operator--()
{
	--m_xmlNodeIterator;
	return *this;
}

ego::XmlNamedNodeIterator ego::XmlNamedNodeIterator::operator--(int)
{
	const XmlNamedNodeIterator tmp = *this;
	--m_xmlNodeIterator;
	return tmp;
}

ego::XmlNode ego::XmlNode::getChild(const char* _name) const
{
	return m_xmlNode.child(_name);
}

ego::XmlNodeRange<ego::XmlNodeIterator> ego::XmlNode::getAllChildren() const
{
	pugi::xml_object_range<pugi::xml_node_iterator> xmlRange = m_xmlNode.children();
	return XmlNodeRange{ XmlNodeIterator(xmlRange.begin()), XmlNodeIterator(xmlRange.end()) };
}

ego::XmlNodeRange<ego::XmlNamedNodeIterator> ego::XmlNode::getChildren(const char* _name) const
{
	pugi::xml_object_range<pugi::xml_named_node_iterator> xmlRange = m_xmlNode.children(_name);
	return XmlNodeRange{ XmlNamedNodeIterator(xmlRange.begin()), XmlNamedNodeIterator(xmlRange.end()) };
}

ego::XmlNode ego::XmlNode::getFirstChild() const
{
	return m_xmlNode.first_child();
}

ego::XmlNode ego::XmlNode::getLastChild() const
{
	return m_xmlNode.last_child();
}

ego::XmlNode ego::XmlNode::getNextSibling() const
{
	return m_xmlNode.next_sibling();
}

ego::XmlNode ego::XmlNode::getPrevSibling() const
{
	return m_xmlNode.previous_sibling();
}

const char* ego::XmlNode::getName() const
{
	return m_xmlNode.name();
}

std::string_view ego::XmlNode::getNameView() const
{
	return getName();
}

bool ego::XmlNode::setName(const char* _name)
{
	return m_xmlNode.set_name(_name);
}

bool ego::XmlNode::hasAttribute(const char* _name) const
{
	return m_xmlNode.attribute(_name);
}

const char* ego::XmlNode::getAttributeValue(const char* _name) const
{
	return m_xmlNode.attribute(_name).value();
}

ego::XmlNodeValue ego::XmlNode::getValue() const
{
	return m_xmlNode.text();
}
