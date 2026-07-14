#pragma once

#include "EgoResource/Resource.h"

#include "EgoGraphicHardware/GraphicObjects/Shader.h"

namespace ego::gpu
{
    class ShaderResource : public Resource
    {
    public:
        ShaderResource() = default;

        ShaderReference getShader() const;

        virtual ShaderStage getShaderStage() const = 0;

        EGO_RESOURCE(ShaderResource, Resource);

    protected:
        bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        void onUnload() override;

        virtual ShaderReference createShader(const ShaderCodeReference& _code) = 0;

        ShaderReference m_shader = nullptr;
    };

    EGO_POINTER(ShaderResource);

    class VertexShaderResource final : public ShaderResource
    {
    public:
        VertexShaderResource() = default;

        VertexShaderReference getVertexShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(VertexShaderResource, ShaderResource);

    protected:
        ShaderReference createShader(const ShaderCodeReference& _code) override;
    };

    EGO_POINTER(VertexShaderResource);

    class PixelShaderResource final : public ShaderResource
    {
    public:
        PixelShaderResource() = default;

        PixelShaderReference getPixelShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(PixelShaderResource, ShaderResource);

    protected:
        ShaderReference createShader(const ShaderCodeReference& _code) override;
    };

    EGO_POINTER(PixelShaderResource);

    class ComputeShaderResource final : public ShaderResource
    {
    public:
        ComputeShaderResource() = default;

        ComputeShaderReference getComputeShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(ComputeShaderResource, ShaderResource);

    protected:
        ShaderReference createShader(const ShaderCodeReference& _code) override;
    };

    EGO_POINTER(ComputeShaderResource);

    class RayGenerationShaderResource final : public ShaderResource
    {
    public:
        RayGenerationShaderResource() = default;

        RayGenerationShaderReference getRayGenerationShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(RayGenerationShaderResource, ShaderResource);

    protected:
        ShaderReference createShader(const ShaderCodeReference& _code) override;
    };

    EGO_POINTER(RayGenerationShaderResource);

    class MissShaderResource final : public ShaderResource
    {
    public:
        MissShaderResource() = default;

        MissShaderReference getMissShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(MissShaderResource, ShaderResource);

    protected:
        ShaderReference createShader(const ShaderCodeReference& _code) override;
    };

    EGO_POINTER(MissShaderResource);

    class ClosestHitShaderResource final : public ShaderResource
    {
    public:
        ClosestHitShaderResource() = default;

        ClosestHitShaderReference getClosestHitShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(ClosestHitShaderResource, ShaderResource);

    protected:
        ShaderReference createShader(const ShaderCodeReference& _code) override;
    };

    EGO_POINTER(ClosestHitShaderResource);

    class AnyHitShaderResource final : public ShaderResource
    {
    public:
        AnyHitShaderResource() = default;

        AnyHitShaderReference getAnyHitShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(AnyHitShaderResource, ShaderResource);

    protected:
        ShaderReference createShader(const ShaderCodeReference& _code) override;
    };

    EGO_POINTER(AnyHitShaderResource);

    class IntersectionShaderResource final : public ShaderResource
    {
    public:
        IntersectionShaderResource() = default;

        IntersectionShaderReference getIntersectionShader() const;
        ShaderStage getShaderStage() const override;

        EGO_RESOURCE(IntersectionShaderResource, ShaderResource);

    protected:
        ShaderReference createShader(const ShaderCodeReference& _code) override;
    };

    EGO_POINTER(IntersectionShaderResource);
} // namespace ego::gpu
