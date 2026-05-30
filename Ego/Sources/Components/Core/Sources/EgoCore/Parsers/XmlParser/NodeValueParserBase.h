#pragma once

#include "NodeValueParserInterface.h"

namespace ego
{
	template <>
	struct XmlValueParser<const char*>
	{
		static void parse(std::string_view _str, const char*& _val)
		{
			_val = _str.data();
		}
	};
}
