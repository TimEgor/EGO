#pragma once

#include <utility>
#include <vector>

#include "EgoCore/Handle/Handle.h"
#include "EgoCore/Reference/Reference.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Pipeline.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/ResourceView.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Sampler.h"

namespace ego
{
    class Material final : public STDDestroyMTCountable
    {
    public:
        using ResourceViewCollection = std::vector<gpu::ResourceViewReference>;
        using SamplerCollection = std::vector<gpu::SamplerReference>;

        Material(
            const gpu::GraphicPipelineReference& _pipeline,
            ResourceViewCollection _resourceViews = ResourceViewCollection(),
            SamplerCollection _samplers = SamplerCollection()
        );

        const gpu::GraphicPipelineReference& getPipeline() const;

        const ResourceViewCollection& getResourceViews() const;

        const SamplerCollection& getSamplers() const;

    private:
        const gpu::GraphicPipelineReference m_pipeline;
        const ResourceViewCollection m_resourceViews;
        const SamplerCollection m_samplers;
    };

    EGO_REFERENCE(Material);

    EGO_HANDLE(Material);

    MaterialHandle CreateMaterialHandle(const MaterialReference& _material);
}
