#pragma once

#include <string>
#include <vector>

#include "EgoEngine/Graphic/Render/RenderObject.h"
#include "EgoEngine/Resources/Resource/Resource.h"

namespace ego
{
    class XmlNode;
} // namespace ego

namespace ego::gpu
{
    class AnyHitShaderResource;
    class ClosestHitShaderResource;
    class IntersectionShaderResource;
    class PixelShaderResource;
    class VertexShaderResource;
} // namespace ego::gpu

namespace ego::render
{
    class MaterialResource final : public Resource
    {
    public:
        MaterialResource() = default;

        const RenderMaterial& getMaterial() const;

        EGO_RESOURCE(MaterialResource, Resource);

    protected:
        bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        void onUnload() override;

    private:
        struct RasterizationPassData final
        {
            std::string m_name;
            SharedPointer<gpu::VertexShaderResource> m_vertexShaderResource = nullptr;
            SharedPointer<gpu::PixelShaderResource> m_pixelShaderResource = nullptr;
        };

        struct RayTracingPassData final
        {
            std::string m_name;
            SharedPointer<gpu::ClosestHitShaderResource> m_closestHitShaderResource = nullptr;
            SharedPointer<gpu::AnyHitShaderResource> m_anyHitShaderResource = nullptr;
            SharedPointer<gpu::IntersectionShaderResource> m_intersectionShaderResource = nullptr;
        };

        using RasterizationPassCollection = std::vector<RasterizationPassData>;
        using RayTracingPassCollection = std::vector<RayTracingPassData>;

        bool loadPasses(const XmlNode& _materialNode, ResourceLoadingContext& _loadingContext);
        bool loadPass(const XmlNode& _passNode, ResourceLoadingContext& _loadingContext);
        bool loadRasterizationPass(const XmlNode& _passNode, const std::string& _passName, ResourceLoadingContext& _loadingContext);
        bool loadRayTracingPass(const XmlNode& _passNode, const std::string& _passName, ResourceLoadingContext& _loadingContext);

        bool buildMaterial();
        bool addRasterizationPassToTemplate(const RasterizationPassData& _passData, const MaterialTemplateReference& _materialTemplate);
        bool addRayTracingPassToTemplate(const RayTracingPassData& _passData, const MaterialTemplateReference& _materialTemplate);
        bool addPassToTemplate(
            const std::string& _passName,
            const MaterialRenderPassInfoReference& _passInfo,
            const MaterialTemplateReference& _materialTemplate);

        bool readPassName(const XmlNode& _passNode, std::string& _name);
        bool readPassType(const XmlNode& _passNode, const std::string& _passName, std::string& _type);
        bool readRequiredFileName(const XmlNode& _node, const std::string& _passName, const char* _childName, FileName& _fileName);

        template <typename TResource>
        bool loadRequiredShaderDependency(
            const XmlNode& _passNode,
            const std::string& _passName,
            const char* _childName,
            ResourceLoadingContext& _loadingContext,
            SharedPointer<TResource>& _resource);

        template <typename TResource>
        SharedPointer<TResource> loadShaderDependency(ResourceLoadingContext& _loadingContext, const FileName& _path);
        template <typename TResource>
        bool loadOptionalShaderDependency(
            const XmlNode& _passNode,
            const std::string& _passName,
            const char* _childName,
            ResourceLoadingContext& _loadingContext,
            SharedPointer<TResource>& _resource);

        void clearPassData();

        RasterizationPassCollection m_rasterizationPasses;
        RayTracingPassCollection m_rayTracingPasses;
        RenderMaterial m_material = nullptr;
    };

    EGO_POINTER(MaterialResource);

    RenderMaterial CreateMaterialHandler(const MaterialResourcePointer& _resource);
} // namespace ego::render
