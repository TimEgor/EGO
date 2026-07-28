#pragma once

#include "EgoEngine/Graphic/SceneRender/MaterialTemplate.h"

namespace ego::render
{
    class Material final : public MTCountable
    {
    public:
        explicit Material(const MaterialTemplatePointer& _materialTemplate);

        const MaterialTemplatePointer& getMaterialTemplate() const;

    private:
        const MaterialTemplatePointer m_materialTemplate = nullptr;
    };

    EGO_INTRUSIVE_POINTER(Material);
    EGO_NAMED_HANDLER(MaterialPointer, Material);
} // namespace ego::render
