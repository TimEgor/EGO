#pragma once

#include "EgoCore/RTTI/RTTI.h"

#include "EgoEngine/Graphic/Render/RenderGpuObject.h"

namespace ego::render
{
    struct RayTracingMaterialHitGroup final
    {
        RenderClosestHitShader m_closestHitShader = nullptr;
        RenderAnyHitShader m_anyHitShader = nullptr;
        RenderIntersectionShader m_intersectionShader = nullptr;

        bool operator==(const RayTracingMaterialHitGroup& _other) const;
        bool operator!=(const RayTracingMaterialHitGroup& _other) const;
    };

    class MaterialRenderPassInfo : public STDDestroyMTCountable
    {
    public:
        MaterialRenderPassInfo() = default;
        ~MaterialRenderPassInfo() override = default;

        EGO_RTTI_VIRTUAL_BASE(MaterialRenderPassInfo);
    };

    EGO_REFERENCE(MaterialRenderPassInfo);

    class RasterizationMaterialRenderPassInfo final : public MaterialRenderPassInfo
    {
    public:
        RasterizationMaterialRenderPassInfo(const RenderVertexShader& _vertexShader, const RenderPixelShader& _pixelShader);

        const RenderVertexShader& getVertexShader() const;
        const RenderPixelShader& getPixelShader() const;

        EGO_RTTI_VIRTUAL(RasterizationMaterialRenderPassInfo, MaterialRenderPassInfo);

    private:
        RenderVertexShader m_vertexShader = nullptr;
        RenderPixelShader m_pixelShader = nullptr;
    };

    EGO_REFERENCE(RasterizationMaterialRenderPassInfo);

    class RayTracingMaterialRenderPassInfo final : public MaterialRenderPassInfo
    {
    public:
        RayTracingMaterialRenderPassInfo(const RayTracingMaterialHitGroup& _hitGroup);

        const RayTracingMaterialHitGroup& getHitGroup() const;

        EGO_RTTI_VIRTUAL(RayTracingMaterialRenderPassInfo, MaterialRenderPassInfo);

    private:
        RayTracingMaterialHitGroup m_hitGroup;
    };

    EGO_REFERENCE(RayTracingMaterialRenderPassInfo);
} // namespace ego::render
