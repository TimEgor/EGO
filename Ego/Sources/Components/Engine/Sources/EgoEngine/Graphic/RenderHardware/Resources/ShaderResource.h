#pragma once

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Shader.h"
#include "EgoEngine/Resources/Resource/Resource.h"

namespace ego::gpu
{
    class ShaderResource : public Resource
    {
    public:
        ShaderResource() = default;

        ShaderPointer getShader() const;

        EGO_RESOURCE(ShaderResource, Resource);

    protected:
        virtual bool onLoad(FileContent&& _content, ResourceLoadingContext& _loadingContext) override;
        virtual void onUnload() override;

        virtual ShaderPointer createShader(const ShaderCodePointer& _code) = 0;

        ShaderPointer m_shader = nullptr;
    };

    EGO_POINTER(ShaderResource);

    class VertexShaderResource final : public ShaderResource
    {
    public:
        VertexShaderResource() = default;

        VertexShaderPointer getVertexShader() const;

        EGO_RESOURCE(VertexShaderResource, ShaderResource);

    protected:
        virtual ShaderPointer createShader(const ShaderCodePointer& _code) override;
    };

    EGO_POINTER(VertexShaderResource);

    class PixelShaderResource final : public ShaderResource
    {
    public:
        PixelShaderResource() = default;

        PixelShaderPointer getPixelShader() const;

        EGO_RESOURCE(PixelShaderResource, ShaderResource);

    protected:
        virtual ShaderPointer createShader(const ShaderCodePointer& _code) override;
    };

    EGO_POINTER(PixelShaderResource);

    class ComputeShaderResource final : public ShaderResource
    {
    public:
        ComputeShaderResource() = default;

        ComputeShaderPointer getComputeShader() const;

        EGO_RESOURCE(ComputeShaderResource, ShaderResource);

    protected:
        virtual ShaderPointer createShader(const ShaderCodePointer& _code) override;
    };

    EGO_POINTER(ComputeShaderResource);
}
