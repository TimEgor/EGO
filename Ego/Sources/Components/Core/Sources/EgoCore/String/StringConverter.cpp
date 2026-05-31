// std has no non-deprecated general UTF converter in C++20; keep this warning
// suppression local until the converter moves to a platform-specific backend.
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

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
