#include "ResourceProvider.h"

#include <utility>

bool ego::ResourceProvider::provideContent(
    const Resource& _resource,
    const FileName& _path,
    ResourceLoadingContext& _loadingContext,
    FileContent& _content,
    std::string& _loadingError
)
{
    std::lock_guard locker(m_mutex);
    m_loadingError.clear();

    const bool result = onProvideContent(_resource, _path, _loadingContext, _content);
    _loadingError = m_loadingError;
    return result;
}

std::string ego::ResourceProvider::getLoadingError() const
{
    std::lock_guard locker(m_mutex);
    return m_loadingError;
}

void ego::ResourceProvider::clearLoadingError()
{
    std::lock_guard locker(m_mutex);
    m_loadingError.clear();
}

void ego::ResourceProvider::setLoadingError(std::string _loadingError)
{
    std::lock_guard locker(m_mutex);
    m_loadingError = std::move(_loadingError);
}
