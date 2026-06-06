#pragma once

#include <utility>
#include <vector>

#include "EgoEngine/Graphic/Render/RenderGpuObject.h"

namespace ego::render
{
    class Material final : public STDDestroyMTCountable
    {
    public:
        using ResourceViewCollection = std::vector<RenderResourceView>;
        using SamplerCollection = std::vector<RenderSampler>;

        Material(
            const RenderGraphicPipeline& _pipeline,
            ResourceViewCollection _resourceViews = ResourceViewCollection(),
            SamplerCollection _samplers = SamplerCollection()
        );

        const RenderGraphicPipeline& getPipeline() const;

        const ResourceViewCollection& getResourceViews() const;

        const SamplerCollection& getSamplers() const;

    private:
        const RenderGraphicPipeline m_pipeline;
        const RenderVertexShader m_vertexShader;
        const RenderPixelShader m_pixelShader;
        const ResourceViewCollection m_resourceViews;
        const SamplerCollection m_samplers;
    };

    EGO_REFERENCE(Material);
    EGO_NAMED_HANDLER(MaterialReference, Material);
}
