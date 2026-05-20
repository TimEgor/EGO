#include "StringConverter.h"

#include <codecvt>
#include <locale>

std::wstring ego::ConvertStringToWString(const std::string& _str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.from_bytes(_str);
}

std::string ego::ConvertWStringToString(const std::wstring& _wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.to_bytes(_wstr);
}
