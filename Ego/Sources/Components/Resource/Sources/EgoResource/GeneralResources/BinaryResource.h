#pragma once

#include "EgoResource/Resource.h"

namespace ego
{
    class BinaryResource final : public Resource
    {
    public:
        BinaryResource() = default;

        const FileContent& getContent() const;

        EGO_RESOURCE(BinaryResource, Resource);

    protected:
        bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        void onUnload() override;

    private:
        FileContent m_content;
    };

    EGO_POINTER(BinaryResource);
} // namespace ego
