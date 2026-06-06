#include "ResourceProvider.h"

bool ego::ResourceProvider::provideContent(
    const Resource& _resource,
    const FileName& _path,
    ResourceLoadingContext& _loadingContext,
    FileContent& _content,
    std::string& _loadingError
)
{
    _loadingError.clear();
    return onProvideContent(_resource, _path, _loadingContext, _content, _loadingError);
}
