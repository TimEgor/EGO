#pragma once

#include <cstdint>

#include "GraphicResource.h"

namespace ego::gpu
{
    enum class ShaderStage
    {
        Vertex,
        Pixel,
        Compute
    };

    enum ShaderStageFlag
    {
        ShaderStageFlagNone = 0,
        ShaderStageFlagVertex = 1 << 0,
        ShaderStageFlagPixel = 1 << 2,
        ShaderStageFlagCompute = 1 << 3,
        ShaderStageFlagAllGraphics = ShaderStageFlagVertex | ShaderStageFlagPixel,
        ShaderStageFlagAll = ShaderStageFlagAllGraphics | ShaderStageFlagCompute
    };

    using ShaderStageFlags = uint32_t;

    class ShaderCode final : public STDDestroyMTCountable
    {
    public:
        ShaderCode(const void* _code, uint32_t _codeSize);
        ~ShaderCode();

        void* getCode() const;
        uint32_t getCodeSize() const;

    private:
        uint8_t* m_code = nullptr;
        uint32_t m_codeSize = 0;
    };

    EGO_REFERENCE(ShaderCode);

    class Shader : public GraphicResource
    {
    public:
        Shader(const ShaderCodeReference& _code);

        ShaderCodeReference getCode() const;

        virtual ShaderStage getShaderType() const = 0;

        EGO_GRAPHIC_RESOURCE(Shader, GraphicResource);

    protected:
        ShaderCodeReference m_code;
    };

    EGO_REFERENCE(Shader);

    class VertexShader : public Shader
    {
    public:
        VertexShader(const ShaderCodeReference& _code);

        virtual ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(VertexShader, Shader);
    };

    EGO_REFERENCE(VertexShader);

    class PixelShader : public Shader
    {
    public:
        PixelShader(const ShaderCodeReference& _code);

        virtual ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(PixelShader, Shader);
    };

    EGO_REFERENCE(PixelShader);

    class ComputeShader : public Shader
    {
    public:
        ComputeShader(const ShaderCodeReference& _code);

        virtual ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(ComputeShader, Shader);
    };

    EGO_REFERENCE(ComputeShader);
}
