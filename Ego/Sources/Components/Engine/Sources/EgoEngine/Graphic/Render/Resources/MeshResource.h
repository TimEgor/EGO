#pragma once

#include "EgoEngine/Graphic/Render/RenderObject.h"
#include "EgoEngine/Resources/Resource/Resource.h"

namespace ego::render
{
    class MeshResource final : public Resource
    {
    public:
        MeshResource() = default;

        const RenderMesh& getMesh() const;

        EGO_RESOURCE(MeshResource, Resource);

    protected:
        virtual bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        virtual void onUnload() override;

    private:
        RenderMesh m_mesh = nullptr;
    };

    EGO_POINTER(MeshResource);

    RenderMesh CreateMeshHandler(const MeshResourcePointer& _resource);
}
