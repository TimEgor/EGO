#pragma once

#include "EgoEngine/Graphic/Render/Material.h"
#include "EgoEngine/Resources/Resource/Resource.h"

namespace ego
{
    class MaterialResource final : public Resource
    {
    public:
        MaterialResource() = default;

        const MaterialReference& getMaterial() const;
        void setMaterial(const MaterialReference& _material);

        EGO_RESOURCE(MaterialResource, Resource);

    protected:
        virtual bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        virtual void onUnload() override;

    private:
        MaterialReference m_material = nullptr;
    };

    EGO_POINTER(MaterialResource);

    MaterialHandle CreateMaterialHandle(const MaterialResourcePointer& _resource);
}
