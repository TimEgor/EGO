#pragma once

#include "EgoEngine/Resources/Resource/Resource.h"

#include <string>

namespace ego
{
    class TextResource final : public Resource
    {
    public:
        TextResource() = default;

        const std::string& getText() const;

        EGO_RESOURCE(TextResource, Resource);

    protected:
        virtual bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        virtual void onUnload() override;

    private:
        std::string m_text;
    };

    EGO_POINTER(TextResource);
}
