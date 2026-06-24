#pragma once

#include "EgoEngine/Graphic/Render/RenderGpuObject.h"

namespace ego::render
{
    class Material final : public STDDestroyMTCountable
    {
    public:
        Material(const RenderVertexShader& _vertexShader, const RenderPixelShader& _pixelShader);

        const RenderVertexShader& getVertexShader() const;
        const RenderPixelShader& getPixelShader() const;

    private:
        const RenderVertexShader m_vertexShader;
        const RenderPixelShader m_pixelShader;
    };

    EGO_REFERENCE(Material);
    EGO_NAMED_HANDLER(MaterialReference, Material);
} // namespace ego::render
