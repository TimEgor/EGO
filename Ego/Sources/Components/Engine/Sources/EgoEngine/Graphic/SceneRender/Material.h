#pragma once

#include "EgoEngine/Graphic/SceneRender/MaterialTemplate.h"

namespace ego::render
{
    class Material final : public STDDestroyMTCountable
    {
    public:
        explicit Material(const MaterialTemplateReference& _materialTemplate);

        const MaterialTemplateReference& getMaterialTemplate() const;

    private:
        const MaterialTemplateReference m_materialTemplate = nullptr;
    };

    EGO_REFERENCE(Material);
    EGO_NAMED_HANDLER(MaterialReference, Material);
} // namespace ego::render
