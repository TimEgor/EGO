#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

#include "EgoEngine/Graphic/SceneRender/MaterialRenderPassInfo.h"

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
        struct RayTracingHitGroupKey final
        {
            gpu::RayTracingHitGroupType m_type = gpu::RayTracingHitGroupType::Triangles;
            const gpu::ClosestHitShader* m_closestHitShader = nullptr;
            const gpu::AnyHitShader* m_anyHitShader = nullptr;
            const gpu::IntersectionShader* m_intersectionShader = nullptr;

            bool operator==(const RayTracingHitGroupKey& _other) const;
            bool operator!=(const RayTracingHitGroupKey& _other) const;
        };

        struct RayTracingHitGroupKeyHash final
        {
            size_t operator()(const RayTracingHitGroupKey& _key) const;
        };

        struct RayTracingHitGroupEntry final
        {
            RayTracingHitGroupKey m_key;
            RayTracingMaterialHitGroup m_hitGroup;
        };

        bool loadShaders();
        void clearHitGroupTable();
        bool buildHitGroupTable(const DefaultRenderScene::ItemCollection& _renderItems);
        bool findHitGroupIndex(const RayTracingMaterialHitGroup& _hitGroup, uint32_t& _index) const;
        RenderRayTracingPipeline getOrCreatePipeline(
            GraphicDevice& _graphicDevice,
            RenderPipelineStateCache& _pipelineStateCache) const;
        static RayTracingHitGroupKey MakeHitGroupKey(const RayTracingMaterialHitGroup& _hitGroup);
        static gpu::RayTracingHitGroupDesc MakeHitGroupDesc(const RayTracingMaterialHitGroup& _hitGroup);

        RenderRayGenerationShader m_rayGenerationShader = nullptr;
        RenderMissShader m_missShader = nullptr;
        RenderBindingLayout m_bindingLayout = nullptr;
        RenderInstanceAccelerationStructure m_sceneAccelerationStructure = nullptr;
        RenderAccelerationStructureView m_sceneAccelerationStructureView = nullptr;
        std::vector<RayTracingHitGroupEntry> m_hitGroupTable;
        std::unordered_map<RayTracingHitGroupKey, uint32_t, RayTracingHitGroupKeyHash> m_hitGroupIndices;
        RayTracingGeometryCache m_geometryCache;
    };
} // namespace ego::render
