#pragma once

#include "EgoDefaultRender/RayTracingGeometryCache.h"
#include "EgoDefaultRender/RenderGraph/RenderPass.h"

namespace ego::render
{
    class RayTracingRenderPass final : public RenderPass
    {
    public:
        RayTracingRenderPass() = default;

        bool init(RenderPassInitContext& _context) override;
        void release() override;
        void clearResources() override;
        void declare(RenderPassBuilder& _builder) override;
        bool prepare(RenderPassPrepareContext& _context) override;
        void execute(RenderPassExecuteContext& _context) override;

    private:
        bool loadShaders();
        bool initPipeline(GraphicDevice& _graphicDevice, const RenderBindingLayout& _bindingLayout);

        RenderRayGenerationShader m_rayGenerationShader = nullptr;
        RenderMissShader m_missShader = nullptr;
        RenderClosestHitShader m_closestHitShader = nullptr;
        RenderBindingLayout m_bindingLayout = nullptr;
        RenderRayTracingPipeline m_pipeline = nullptr;
        RenderInstanceAccelerationStructure m_sceneAccelerationStructure = nullptr;
        RenderAccelerationStructureView m_sceneAccelerationStructureView = nullptr;
        RayTracingGeometryCache m_geometryCache;
    };
} // namespace ego::render
