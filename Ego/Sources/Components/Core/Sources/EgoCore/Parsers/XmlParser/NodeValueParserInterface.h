#pragma once

#include <exception>
#include <string_view>

#include "EgoCore/Parsers/TextValueParser/TextValueParser.h"

namespace ego
{
	template <typename T>
	struct XmlValueParser
	{
		static void parse(std::string_view _str, T& _val)
		{
			ParseTextValue(_str, _val);
		}
	};

	template <typename T>
	void ParseXmlValue(std::string_view _str, T& _val)
	{
		XmlValueParser<T>::parse(_str, _val);
	}

	template <typename T>
	bool TryParseXmlValue(std::string_view _str, T& _val)
	{
		try
		{
			ParseXmlValue(_str, _val);
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	class XmlNode;
}

namespace ego::xml_parser_interface
{
	template <typename T>
	void GetValue(const XmlNode& _node, T& _val);

	template <typename T>
	void GetValue(const XmlNode& _node, T* _val);

	template <typename T>
	void SetValue(XmlNode& _node, const T& _val);

	template <typename T>
	void SetValue(XmlNode& _node, const T* _val);
}
