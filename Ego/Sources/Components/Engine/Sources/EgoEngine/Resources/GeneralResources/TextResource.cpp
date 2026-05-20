#include "TextResource.h"

const std::string& ego::TextResource::getText() const
{
    return m_text;
}

bool ego::TextResource::onLoad(FileContent&& _content, ResourceLoadingContext&)
{
    if (_content.empty())
    {
        m_text.clear();
        return true;
    }

    m_text.assign(reinterpret_cast<const char*>(_content.data()), _content.size());
    return true;
}

void ego::TextResource::onUnload()
{
    m_text.clear();
}
