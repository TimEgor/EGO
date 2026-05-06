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

    class ShaderCode final
    {
    public:
        ShaderCode(void* _code, uint32_t _codeSize);
        ~ShaderCode();

        void* getCode() const;
        uint32_t getCodeSize() const;

    private:
        void* m_code = nullptr;
        uint32_t m_codeSize = 0;
    };

    EGO_POINTER(ShaderCode);

    class Shader : public GraphicResource
    {
    public:
        Shader(const ShaderCodePointer& _code);

        ShaderCodePointer getCode() const;

        virtual ShaderStage getShaderType() const = 0;

        EGO_GRAPHIC_RESOURCE(Shader, GraphicResource);

    protected:
        ShaderCodePointer m_code;
    };

    EGO_POINTER(Shader);

    class VertexShader : public Shader
    {
    public:
        VertexShader(const ShaderCodePointer& _code);

        virtual ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(VertexShader, Shader);
    };

    EGO_POINTER(VertexShader);

    class PixelShader : public Shader
    {
    public:
        PixelShader(const ShaderCodePointer& _code);

        virtual ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(PixelShader, Shader);
    };

    EGO_POINTER(PixelShader);

    class ComputeShader : public Shader
    {
    public:
        ComputeShader(const ShaderCodePointer& _code);

        virtual ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(ComputeShader, Shader);
    };

    EGO_POINTER(ComputeShader);
}
