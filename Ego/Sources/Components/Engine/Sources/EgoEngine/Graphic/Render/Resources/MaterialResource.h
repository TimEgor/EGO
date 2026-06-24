#pragma once

#include "EgoEngine/Graphic/Render/RenderObject.h"
#include "EgoEngine/Resources/Resource/Resource.h"

namespace ego::gpu
{
    class PixelShaderResource;
    class VertexShaderResource;
} // namespace ego::gpu

namespace ego::render
{
    class MaterialResource final : public Resource
    {
    public:
        MaterialResource() = default;

        const RenderMaterial& getMaterial() const;

        EGO_RESOURCE(MaterialResource, Resource);

    protected:
        bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        void onUnload() override;

    private:
        bool completeLoading(const SharedPointer<gpu::VertexShaderResource>& _vertexShaderResource, const SharedPointer<gpu::PixelShaderResource>& _pixelShaderResource);

        RenderMaterial m_material = nullptr;
    };

    EGO_POINTER(MaterialResource);

    RenderMaterial CreateMaterialHandler(const MaterialResourcePointer& _resource);
} // namespace ego::render
