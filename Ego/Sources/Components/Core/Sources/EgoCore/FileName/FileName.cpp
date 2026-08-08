#include "FileName.h"

#include <cctype>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Hash/CRC32.h"

#define EGO_FILE_NAME_LOWERCASE 0

ego::FileName::FileName(StringView _name)
{
    assign(_name);
}

ego::FileName::FileName(CharType _character)
{
    operator=(_character);
}

ego::FileName& ego::FileName::operator=(StringView _name)
{
    assign(_name);

    return *this;
}

ego::FileName& ego::FileName::operator=(CharType _character)
{
    assign(StringView(&_character, 1));

    return *this;
}

ego::FileName& ego::FileName::operator+=(StringView _name)
{
    append(_name);

    return *this;
}

ego::FileName& ego::FileName::operator+=(CharType _character)
{
    append(StringView(&_character, 1));

    return *this;
}

ego::FileName& ego::FileName::operator+=(const FileName& _name)
{
    return operator+=(_name.getView());
}

bool ego::FileName::operator==(const FileName& _name) const
{
    return m_name == _name.m_name;
}

ego::FileName::operator bool() const
{
    return !empty();
}

const ego::FileName::CharType& ego::FileName::operator[](size_t _index) const
{
    return m_name[_index];
}

ego::FileName::CharType& ego::FileName::operator[](size_t _index)
{
    return m_name[_index];
}

ego::FileName::StringView ego::FileName::getView() const
{
    return StringView(m_name.data(), m_name.size());
}

const ego::FileName::CharType* ego::FileName::c_str() const
{
    return m_name.c_str();
}

size_t ego::FileName::length() const
{
    return m_name.size();
}

bool ego::FileName::empty() const
{
    return m_name.empty();
}

void ego::FileName::clear()
{
    m_name.clear();
}

void ego::FileName::release()
{
    m_name = StringType();
}

ego::FileNameID ego::FileName::hash() const
{
    const StringView view = getView();

    return Crc32(view.data(), view.size());
}

bool ego::FileName::IsValid(StringView _name)
{
    return _name.find('\0') == StringView::npos;
}

void ego::FileName::assign(StringView _name)
{
    const bool isValid = IsValid(_name);
    EGO_ASSERT(isValid);
    if (!isValid)
    {
        return;
    }

    if (_name.empty())
    {
        m_name.clear();

        return;
    }

    m_name.assign(_name.data(), _name.size());
    normalize(0);
}

void ego::FileName::append(StringView _name)
{
    const bool isValid = IsValid(_name);
    EGO_ASSERT(isValid);
    if (!isValid || _name.empty())
    {
        return;
    }

    const size_t beginIndex = m_name.size();
    m_name.append(_name.data(), _name.size());
    normalize(beginIndex);
}

void ego::FileName::normalize(size_t _beginIndex)
{
#if EGO_FILE_NAME_LOWERCASE
    const size_t size = m_name.size();
    for (size_t index = _beginIndex; index < size; ++index)
    {
        m_name[index] = static_cast<CharType>(std::tolower(static_cast<unsigned char>(m_name[index])));
    }
#endif
}

ego::FileName ego::operator+(FileName _name, FileName::StringView _suffix)
{
    _name += _suffix;

    return _name;
}

ego::FileName ego::operator+(FileName::StringView _prefix, const FileName& _name)
{
    FileName result(_prefix);
    result += _name;

    return result;
}

ego::FileName ego::operator+(FileName _name, FileName::CharType _character)
{
    _name += _character;

    return _name;
}

ego::FileName ego::operator+(FileName::CharType _character, const FileName& _name)
{
    FileName result(_character);
    result += _name;

    return result;
}

ego::FileName ego::operator+(FileName _left, const FileName& _right)
{
    _left += _right;

    return _left;
}
