#pragma once

#include <vector>

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Pipeline.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/ResourceView.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Sampler.h"
#include "EgoEngine/Resources/Resource/Resource.h"

namespace ego
{
    class MaterialResource final : public Resource
    {
    public:
        MaterialResource() = default;

        const gpu::GraphicPipelinePointer& getPipeline() const;
        void setPipeline(const gpu::GraphicPipelinePointer& _pipeline);

        const std::vector<gpu::ResourceViewPointer>& getResourceViews() const;
        void addResourceView(const gpu::ResourceViewPointer& _resourceView);
        void clearResourceViews();

        const std::vector<gpu::SamplerPointer>& getSamplers() const;
        void addSampler(const gpu::SamplerPointer& _sampler);
        void clearSamplers();

        EGO_RESOURCE(MaterialResource, Resource);

    protected:
        virtual bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        virtual void onUnload() override;

    private:
        gpu::GraphicPipelinePointer m_pipeline = nullptr;
        std::vector<gpu::ResourceViewPointer> m_resourceViews;
        std::vector<gpu::SamplerPointer> m_samplers;
    };

    EGO_POINTER(MaterialResource);
}
