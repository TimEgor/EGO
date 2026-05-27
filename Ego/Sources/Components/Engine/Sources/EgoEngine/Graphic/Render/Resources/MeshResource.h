#pragma once

#include "EgoEngine/Graphic/Render/Mesh.h"
#include "EgoEngine/Resources/Resource/Resource.h"

namespace ego
{
    class MeshResource final : public Resource
    {
    public:
        MeshResource() = default;

        const MeshReference& getMesh() const;
        void setMesh(const MeshReference& _mesh);

        EGO_RESOURCE(MeshResource, Resource);

    protected:
        virtual bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        virtual void onUnload() override;

    private:
        MeshReference m_mesh = nullptr;
    };

    EGO_POINTER(MeshResource);

    MeshHandle CreateMeshHandle(const MeshResourcePointer& _resource);
}
