#include "XmlResource.h"

const ego::XmlDocument* ego::XmlResource::getDocument() const
{
    return m_document.get();
}

ego::XmlNode ego::XmlResource::getRootNode() const
{
    return m_document
        ? m_document->getRootNode()
        : XmlNode();
}

bool ego::XmlResource::onLoad(FileContent&& _content, ResourceLoadingContext&)
{
    if (_content.empty())
    {
        m_document.reset();
        return false;
    }

    SharedPointer<XmlDocument> document(new XmlDocument());
    if (!document->loadFromBuffer(_content.data(), _content.size()))
    {
        m_document.reset();
        return false;
    }

    m_document = document;
    return true;
}

void ego::XmlResource::onUnload()
{
    m_document.reset();
}
