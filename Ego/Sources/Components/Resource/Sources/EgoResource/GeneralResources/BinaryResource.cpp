#include "BinaryResource.h"

#include <utility>

const ego::FileContent& ego::BinaryResource::getContent() const
{
    return m_content;
}

bool ego::BinaryResource::onLoad(FileContent&& _content, ResourceLoadingContext&)
{
    m_content = std::move(_content);
    return true;
}

void ego::BinaryResource::onUnload()
{
    m_content.clear();
}
