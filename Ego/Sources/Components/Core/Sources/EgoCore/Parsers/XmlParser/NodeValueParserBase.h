#pragma once

#include <cstring>
#include <string>

#include "EgoCore/FileName/FileName.h"

#include "NodeValueParserInterface.h"

namespace ego::xml_parser_interface
{
	template <>
	inline void GetValue(const XmlNode& _node, bool& _val)
	{
		_val = _node.getValue().get<bool>();
	}

	template <>
	inline void GetValue(const XmlNode& _node, unsigned& _val)
	{
		_val = _node.getValue().get<unsigned>();
	}

	template <>
	inline void GetValue(const XmlNode& _node, int& _val)
	{
		_val = _node.getValue().get<int>();
	}

	template <>
	inline void GetValue(const XmlNode& _node, float& _val)
	{
		_val = _node.getValue().get<float>();
	}

	template <>
	inline void GetValue(const XmlNode& _node, double& _val)
	{
		_val = _node.getValue().get<double>();
	}

	template <>
	inline void GetValue(const XmlNode& _node, char** _val)
	{
		std::strcpy(*_val, _node.getValue().get<const char*>());
	}

	template <>
	inline void GetValue(const XmlNode& _node, std::string& _val)
	{
		_val = _node.getValue().get<const char*>();
	}

	template <>
	inline void GetValue(const XmlNode& _node, FileName& _val)
	{
		_val = _node.getValue().get<const char*>();
	}

	template <>
	inline void SetValue(XmlNode& _node, const bool& _val)
	{
		_node.getValue().set(_val);
	}

	template <>
	inline void SetValue(XmlNode& _node, const unsigned& _val)
	{
		_node.getValue().set(_val);
	}

	template <>
	inline void SetValue(XmlNode& _node, const int& _val)
	{
		_node.getValue().set(_val);
	}

	template <>
	inline void SetValue(XmlNode& _node, const float& _val)
	{
		_node.getValue().set(_val);
	}

	template <>
	inline void SetValue(XmlNode& _node, const double& _val)
	{
		_node.getValue().set(_val);
	}

	template <>
	inline void SetValue(XmlNode& _node, const char* _val)
	{
		_node.getValue().set(_val);
	}

	template <>
	inline void SetValue(XmlNode& _node, const std::string& _val)
	{
		_node.getValue().set(_val.c_str());
	}

	template <>
	inline void SetValue(XmlNode& _node, const FileName& _val)
	{
		_node.getValue().set(_val.c_str());
	}
}
