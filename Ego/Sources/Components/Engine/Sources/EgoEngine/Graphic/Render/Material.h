#pragma once

#include "EgoEngine/Graphic/Render/RenderGpuObject.h"

namespace ego::render
{
    class Material final : public STDDestroyMTCountable
    {
    public:
        Material(const RenderGraphicPipeline& _pipeline);

        const RenderGraphicPipeline& getPipeline() const;

    private:
        const RenderGraphicPipeline m_pipeline;
        const RenderVertexShader m_vertexShader;
        const RenderPixelShader m_pixelShader;
    };

    EGO_REFERENCE(Material);
    EGO_NAMED_HANDLER(MaterialReference, Material);
}
