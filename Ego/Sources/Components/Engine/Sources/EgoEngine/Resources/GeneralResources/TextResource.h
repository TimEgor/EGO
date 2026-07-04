#pragma once

#include <string>

#include "EgoRuntime/Resource/Resource.h"

namespace ego
{
    class TextResource final : public Resource
    {
    public:
        TextResource() = default;

        const std::string& getText() const;

        EGO_RESOURCE(TextResource, Resource);

    protected:
        bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        void onUnload() override;

    private:
        std::string m_text;
    };

    EGO_POINTER(TextResource);
} // namespace ego
