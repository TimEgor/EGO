#include "FileName.h"

#include "EgoCore/Hash/CRC32.h"

#define EGO_FILE_NAME_LOWERCASE FALSE

void ego::FileName::assign(const ResourceNameCharType* _name)
{
    m_name = _name;

#if EGO_FILE_NAME_LOWERCASE
    size_t newSize = m_name.size();
    for (size_t i = 0; i < newSize; ++i)
    {
        m_name[i] = std::tolower(m_name[i]);
    }
#endif
}

ego::FileName::FileName(const ResourceNameCharType* _newName)
{
    assign(_newName);
}

ego::FileName::FileName(ResourceNameCharType _ch)
{
#if EGO_FILE_NAME_LOWERCASE
    _ch = std::tolower(_ch);
#endif

    m_name = _ch;
}

ego::FileName::FileName(const std::string& _str)
    : FileName(_str.c_str())
{
}

ego::FileName::FileName(const FileName& _newName)
    : FileName(_newName.m_name)
{
}

ego::FileName& ego::FileName::operator=(const ResourceNameCharType* _name)
{
    assign(_name);
    return *this;
}

ego::FileName& ego::FileName::operator=(ResourceNameCharType _ch)
{
#if EGO_FILE_NAME_LOWERCASE
    m_name[0] = tolower(_ch);
#else
    m_name[0] = _ch;
#endif
    m_name[1] = '\0';

    return *this;
}

ego::FileName& ego::FileName::operator=(const std::string& _str)
{
    return operator=(_str.c_str());
}

ego::FileName& ego::FileName::operator=(const FileName& _newName)
{
    return operator=(_newName.m_name.c_str());
}

ego::FileName& ego::FileName::operator+=(const ResourceNameCharType* _name)
{
#if EGO_FILE_NAME_LOWERCASE
    size_t baseIndex = m_name.size();

    m_name += _name;

    size_t newSize = m_name.size();
    for (size_t i = baseIndex; i < newSize; ++i)
    {
        m_name[i] = std::tolower(newName[i]);
    }

#else
    m_name += _name;
#endif

    return *this;
}

ego::FileName& ego::FileName::operator+=(ResourceNameCharType _ch)
{
    m_name += _ch;
    return *this;
}

ego::FileName& ego::FileName::operator+=(const std::string& _str)
{
    return operator+=(_str.c_str());
}

ego::FileName& ego::FileName::operator+=(const FileName& _name)
{
    return operator+=(_name.m_name);
}

bool ego::FileName::operator==(const ResourceNameCharType* _name) const
{
    return m_name == _name;
}

bool ego::FileName::operator==(const std::string& _str) const
{
    return m_name == _str;
}

bool ego::FileName::operator==(const FileName& _name) const
{
    return m_name == _name.m_name;
}

bool ego::FileName::operator!=(const ResourceNameCharType* _name) const
{
    return m_name != _name;
}

bool ego::FileName::operator!=(const std::string& _str) const
{
    return m_name != _str;
}

bool ego::FileName::operator!=(const FileName& _name) const
{
    return m_name != _name.m_name;
}

const ego::FileName::ResourceNameCharType& ego::FileName::operator[](size_t _index) const
{
    return m_name[_index];
}

ego::FileName::ResourceNameCharType& ego::FileName::operator[](size_t _index)
{
    return m_name[_index];
}

const ego::FileName::ResourceNameCharType* ego::FileName::c_str() const
{
    return m_name.c_str();
}

bool ego::FileName::empty() const
{
    return m_name.empty();
}

size_t ego::FileName::length() const
{
    return m_name.size();
}

ego::FileNameID ego::FileName::hash() const
{
    return Crc32(m_name.c_str());
}

void ego::FileName::clear()
{
    m_name.clear();
}

void ego::FileName::release()
{
    m_name = ResourceNameContainerType();
}

ego::FileName ego::operator+(const FileName& _resName, const FileName::ResourceNameCharType* _name)
{
    FileName newName(_resName);
    newName += _name;
    return newName;
}

ego::FileName ego::operator+(const FileName::ResourceNameCharType* _name, const FileName& _resName)
{
    FileName newName(_name);
    newName += _resName;
    return newName;
}

ego::FileName ego::operator+(const FileName& _resName, FileName::ResourceNameCharType _ch)
{
    FileName newName(_resName);
    newName += _ch;
    return newName;
}

ego::FileName ego::operator+(FileName::ResourceNameCharType _ch, const FileName& _resName)
{
    FileName newName(_ch);
    newName += _resName;
    return newName;
}

ego::FileName ego::operator+(const FileName& _resName, const std::string& _str)
{
    FileName newName(_resName);
    newName += _str;
    return newName;
}

ego::FileName ego::operator+(const std::string& _str, const FileName& _resName)
{
    FileName newName(_str);
    newName += _resName;
    return newName;
}

ego::FileName ego::operator+(const FileName& _resName1, const FileName& _resName2)
{
    FileName newName(_resName1);
    newName += _resName2;
    return newName;
}

void parseValue(const char* _strVal, ego::FileName& _value)
{
    _value = _strVal;
}