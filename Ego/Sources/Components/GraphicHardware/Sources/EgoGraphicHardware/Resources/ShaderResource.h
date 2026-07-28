#pragma once

#include "EgoResource/Resource.h"

#include "EgoGraphicHardware/GraphicObjects/Shader.h"

namespace ego::gpu
{
    class ShaderResource : public Resource
    {
    public:
        ShaderResource() = default;

        ShaderPointer getShader() const;

        virtual ShaderStage getShaderStage() const = 0;

        EGO_RESOURCE(ShaderResource, Resource);

    protected:
        bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        void onUnload() override;

        virtual ShaderPointer createShader(const ShaderCodePointer& _code) = 0;

        ShaderPointer m_shader = nullptr;
    };

    EGO_POINTER(ShaderResource);

    class VertexShaderResource final : public ShaderResource
    {
    public:
        VertexShaderResource() = default;

        VertexShaderPointer getVertexShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(VertexShaderResource, ShaderResource);

    protected:
        ShaderPointer createShader(const ShaderCodePointer& _code) override;
    };

    EGO_POINTER(VertexShaderResource);

    class PixelShaderResource final : public ShaderResource
    {
    public:
        PixelShaderResource() = default;

        PixelShaderPointer getPixelShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(PixelShaderResource, ShaderResource);

    protected:
        ShaderPointer createShader(const ShaderCodePointer& _code) override;
    };

    EGO_POINTER(PixelShaderResource);

    class ComputeShaderResource final : public ShaderResource
    {
    public:
        ComputeShaderResource() = default;

        ComputeShaderPointer getComputeShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(ComputeShaderResource, ShaderResource);

    protected:
        ShaderPointer createShader(const ShaderCodePointer& _code) override;
    };

    EGO_POINTER(ComputeShaderResource);

    class RayGenerationShaderResource final : public ShaderResource
    {
    public:
        RayGenerationShaderResource() = default;

        RayGenerationShaderPointer getRayGenerationShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(RayGenerationShaderResource, ShaderResource);

    protected:
        ShaderPointer createShader(const ShaderCodePointer& _code) override;
    };

    EGO_POINTER(RayGenerationShaderResource);

    class MissShaderResource final : public ShaderResource
    {
    public:
        MissShaderResource() = default;

        MissShaderPointer getMissShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(MissShaderResource, ShaderResource);

    protected:
        ShaderPointer createShader(const ShaderCodePointer& _code) override;
    };

    EGO_POINTER(MissShaderResource);

    class ClosestHitShaderResource final : public ShaderResource
    {
    public:
        ClosestHitShaderResource() = default;

        ClosestHitShaderPointer getClosestHitShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(ClosestHitShaderResource, ShaderResource);

    protected:
        ShaderPointer createShader(const ShaderCodePointer& _code) override;
    };

    EGO_POINTER(ClosestHitShaderResource);

    class AnyHitShaderResource final : public ShaderResource
    {
    public:
        AnyHitShaderResource() = default;

        AnyHitShaderPointer getAnyHitShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(AnyHitShaderResource, ShaderResource);

    protected:
        ShaderPointer createShader(const ShaderCodePointer& _code) override;
    };

    EGO_POINTER(AnyHitShaderResource);

    class IntersectionShaderResource final : public ShaderResource
    {
    public:
        IntersectionShaderResource() = default;

        IntersectionShaderPointer getIntersectionShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(IntersectionShaderResource, ShaderResource);

    protected:
        ShaderPointer createShader(const ShaderCodePointer& _code) override;
    };

    EGO_POINTER(IntersectionShaderResource);
} // namespace ego::gpu
