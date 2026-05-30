#pragma once

#include <cstddef>
#include <utility>

#include "XmlNode.h"

namespace ego
{
	class FileName;

	class XmlDocument final
	{
	private:
		pugi::xml_document m_xmlDocument;

	public:
		XmlDocument() = default;
		XmlDocument(pugi::xml_document&& _pugiXmlDoc)
			: m_xmlDocument(std::move(_pugiXmlDoc)) {}
		
		bool loadFromFile(const FileName& _file);
		bool loadFromBuffer(const void* _data, size_t _dataSize);
		bool loadFromString(const char* _str);

		XmlNode getDocumentNode() const;
		XmlNode getRootNode() const;
	};
}
