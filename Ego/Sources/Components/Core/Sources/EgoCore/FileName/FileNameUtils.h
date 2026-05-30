#pragma once

#include "FileName.h"
#include "EgoCore/Parsers/ArgParser/ValueParserInterface.h"
#include "EgoCore/Parsers/TextValueParser/TextValueParser.h"

namespace ego::file_name_utils
{
	void GetFileName(const FileName& _path, FileName& _resultName);
	FileName GetFileName(const FileName& _path);

	void GetFileDirPath(const FileName& _path, FileName& _resultPath);
	FileName GetFileDirPath(const FileName& _path);

	void GetFileExtension(const FileName& _path, FileName& _resultExtension);
	FileName GetFileExtension(const FileName& _path);

	void RemoveExtension(FileName& _name);
	FileName RemoveExtension(const FileName& _name);
}

namespace ego
{
    template <>
    struct TextValueParser<FileName>
    {
        static void parse(std::string_view _str, FileName& _val)
        {
            _val = _str.empty() ? std::string() : std::string(_str.data(), _str.size());
        }
    };
}

namespace ego::arg_parser_interface
{
    inline void ParseValue(const char* _str, FileName& _val)
    {
        ParseArgValue(_str != nullptr ? std::string_view(_str) : std::string_view(), _val);
    }
}
