#pragma once

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Shader.h"
#include "EgoEngine/Resources/Resource/Resource.h"

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
        virtual bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        virtual void onUnload() override;

        virtual ShaderReference createShader(const ShaderCodeReference& _code) = 0;

        ShaderReference m_shader = nullptr;
    };

    EGO_POINTER(ShaderResource);

    class VertexShaderResource final : public ShaderResource
    {
    public:
        VertexShaderResource() = default;

        VertexShaderReference getVertexShader() const;
        virtual ShaderStage getShaderStage() const override;

        EGO_RESOURCE(VertexShaderResource, ShaderResource);

    protected:
        virtual ShaderReference createShader(const ShaderCodeReference& _code) override;
    };

    EGO_POINTER(VertexShaderResource);

    class PixelShaderResource final : public ShaderResource
    {
    public:
        PixelShaderResource() = default;

        PixelShaderReference getPixelShader() const;
        virtual ShaderStage getShaderStage() const override;

        EGO_RESOURCE(PixelShaderResource, ShaderResource);

    protected:
        virtual ShaderReference createShader(const ShaderCodeReference& _code) override;
    };

    EGO_POINTER(PixelShaderResource);

    class ComputeShaderResource final : public ShaderResource
    {
    public:
        ComputeShaderResource() = default;

        ComputeShaderReference getComputeShader() const;
        virtual ShaderStage getShaderStage() const override;

        EGO_RESOURCE(ComputeShaderResource, ShaderResource);

    protected:
        virtual ShaderReference createShader(const ShaderCodeReference& _code) override;
    };

    EGO_POINTER(ComputeShaderResource);
}
