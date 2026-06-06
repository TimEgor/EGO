#pragma once

#include "EgoEngine/Graphic/Render/RenderObject.h"
#include "EgoEngine/Resources/Resource/Resource.h"

namespace ego::render
{
    class MaterialResource final : public Resource
    {
    public:
        MaterialResource() = default;

        const RenderMaterial& getMaterial() const;

        EGO_RESOURCE(MaterialResource, Resource);

    protected:
        virtual bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        virtual void onUnload() override;

    private:
        RenderMaterial m_material = nullptr;
    };

    EGO_POINTER(MaterialResource);

    RenderMaterial CreateMaterialHandler(const MaterialResourcePointer& _resource);
}
