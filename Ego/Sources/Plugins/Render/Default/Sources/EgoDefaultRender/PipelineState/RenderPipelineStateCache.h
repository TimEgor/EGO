#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>

#include "EgoCore/Pointer/PointerIdentity.h"

#include "EgoEngine/Graphic/SceneRender/RenderGpuObject.h"

namespace ego
{
    class GraphicDevice;
}

namespace ego::render
{
    class RenderPipelineStateCache final
    {
    public:
        RenderPipelineStateCache() = default;

        RenderGraphicPipeline getOrCreateGraphicPipeline(GraphicDevice& _graphicDevice, const gpu::GraphicPipelineDesc& _desc);
        RenderRayTracingPipeline getOrCreateRayTracingPipeline(GraphicDevice& _graphicDevice, const gpu::RayTracingPipelineDesc& _desc);

        void releaseUnused();
        void clear();

        size_t getGraphicPipelineCount() const;
        size_t getRayTracingPipelineCount() const;

    private:
        struct GraphicPipelineEntry final
        {
            gpu::GraphicPipelineDesc m_desc;
            RenderGraphicPipeline m_pipeline = nullptr;
        };

        struct RayTracingPipelineEntry final
        {
            gpu::RayTracingPipelineDesc m_desc;
            RenderRayTracingPipeline m_pipeline = nullptr;
        };

        using GraphicPipelineMap = std::unordered_multimap<uint64_t, GraphicPipelineEntry>;
        using RayTracingPipelineMap = std::unordered_multimap<uint64_t, RayTracingPipelineEntry>;

        static uint64_t HashRasterizationState(const gpu::RasterizationStateDesc& _desc);
        static uint64_t HashDepthStencilState(const gpu::DepthStencilStateDesc& _desc);
        static uint64_t HashRenderTargetBlend(const gpu::RenderTargetBlendDesc& _desc);
        static uint64_t HashBlendState(const gpu::BlendStateDesc& _desc);
        static uint64_t HashGraphicPipelineDesc(const gpu::GraphicPipelineDesc& _desc);
        static uint64_t HashRayTracingPipelineDesc(const gpu::RayTracingPipelineDesc& _desc);

        static bool IsPipelineHandlerUsedOnlyByCache(const RenderGraphicPipeline& _pipeline);
        static bool IsPipelineHandlerUsedOnlyByCache(const RenderRayTracingPipeline& _pipeline);

        GraphicPipelineMap m_graphicPipelines;
        RayTracingPipelineMap m_rayTracingPipelines;
    };
} // namespace ego::render
