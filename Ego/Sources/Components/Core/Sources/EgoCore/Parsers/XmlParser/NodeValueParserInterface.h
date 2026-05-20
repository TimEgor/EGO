#pragma once

#include "XmlNode.h"

namespace ego::xml_parser_interface
{
	template <typename T>
	void GetValue(const XmlNode& _node, T& _val)
	{
		static_assert(sizeof(T) == 0, "Invalid type for XML parser");
	}

	template <typename T>
	void GetValue(const XmlNode& _node, T* _val)
	{
		static_assert(sizeof(T) == 0, "Invalid type for XML parser");
	}

	template <typename T>
	void SetValue(XmlNode& _node, const T& _val)
	{
		static_assert(sizeof(T) == 0, "Invalid type for XML parser");
	}

	template <typename T>
	void SetValue(XmlNode& _node, const T* _val)
	{
		static_assert(sizeof(T) == 0, "Invalid type for XML parser");
	}
}
