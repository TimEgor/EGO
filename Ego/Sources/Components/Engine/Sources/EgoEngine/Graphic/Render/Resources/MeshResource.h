#pragma once

#include "EgoResource/Resource.h"

#include "EgoEngine/Graphic/Render/RenderObject.h"

namespace ego::render
{
    class MeshResource final : public Resource
    {
    public:
        MeshResource() = default;

        const RenderMesh& getMesh() const;

        EGO_RESOURCE(MeshResource, Resource);

    protected:
        bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        void onUnload() override;

    private:
        RenderMesh m_mesh = nullptr;
    };

    EGO_POINTER(MeshResource);

    RenderMesh CreateMeshHandler(const MeshResourcePointer& _resource);
} // namespace ego::render
