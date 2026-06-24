#include "XmlDocument.h"

#include "EgoCore/FileName/FileName.h"

bool ego::XmlDocument::loadFromFile(const FileName& _file)
{
    return m_xmlDocument.load_file(_file.c_str());
}

bool ego::XmlDocument::loadFromBuffer(const void* _data, size_t _dataSize)
{
    return m_xmlDocument.load_buffer(_data, _dataSize);
}

bool ego::XmlDocument::loadFromString(const char* _str)
{
    return m_xmlDocument.load_string(_str);
}

ego::XmlNode ego::XmlDocument::getDocumentNode() const
{
    return m_xmlDocument;
}

ego::XmlNode ego::XmlDocument::getRootNode() const
{
    return m_xmlDocument.document_element();
}
