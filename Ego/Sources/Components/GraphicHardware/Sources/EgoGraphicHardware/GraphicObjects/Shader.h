#pragma once

#include <cstdint>

#include "EgoCore/Pointer/IntrusivePointer.h"

#include "GraphicResource.h"

namespace ego::gpu
{
    enum class ShaderStage
    {
        Vertex,
        Pixel,
        Compute,

        RayGeneration,
        Miss,
        ClosestHit,
        AnyHit,
        Intersection
    };

    enum ShaderStageFlag
    {
        ShaderStageFlagNone = 0,
        ShaderStageFlagVertex = 1 << 0,
        ShaderStageFlagPixel = 1 << 2,
        ShaderStageFlagCompute = 1 << 3,
        ShaderStageFlagRayGeneration = 1 << 4,
        ShaderStageFlagMiss = 1 << 5,
        ShaderStageFlagClosestHit = 1 << 6,
        ShaderStageFlagAnyHit = 1 << 7,
        ShaderStageFlagIntersection = 1 << 8,
        ShaderStageFlagAllGraphics = ShaderStageFlagVertex | ShaderStageFlagPixel,
        ShaderStageFlagAllRayTracing =
            ShaderStageFlagRayGeneration | ShaderStageFlagMiss | ShaderStageFlagClosestHit | ShaderStageFlagAnyHit | ShaderStageFlagIntersection,
        ShaderStageFlagAll = ShaderStageFlagAllGraphics | ShaderStageFlagCompute | ShaderStageFlagAllRayTracing
    };

    using ShaderStageFlags = uint32_t;

    class ShaderCode final : public MTCountable
    {
    public:
        ShaderCode(const void* _code, uint32_t _codeSize);
        ~ShaderCode() override;

        void* getCode() const;
        uint32_t getCodeSize() const;

    private:
        uint8_t* m_code = nullptr;
        uint32_t m_codeSize = 0;
    };

    EGO_INTRUSIVE_POINTER(ShaderCode);

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

        ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(VertexShader, Shader);
    };

    EGO_POINTER(VertexShader);

    class PixelShader : public Shader
    {
    public:
        PixelShader(const ShaderCodePointer& _code);

        ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(PixelShader, Shader);
    };

    EGO_POINTER(PixelShader);

    class ComputeShader : public Shader
    {
    public:
        ComputeShader(const ShaderCodePointer& _code);

        ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(ComputeShader, Shader);
    };

    EGO_POINTER(ComputeShader);

    class RayGenerationShader : public Shader
    {
    public:
        RayGenerationShader(const ShaderCodePointer& _code);

        ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(RayGenerationShader, Shader);
    };

    EGO_POINTER(RayGenerationShader);

    class MissShader : public Shader
    {
    public:
        MissShader(const ShaderCodePointer& _code);

        ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(MissShader, Shader);
    };

    EGO_POINTER(MissShader);

    class ClosestHitShader : public Shader
    {
    public:
        ClosestHitShader(const ShaderCodePointer& _code);

        ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(ClosestHitShader, Shader);
    };

    EGO_POINTER(ClosestHitShader);

    class AnyHitShader : public Shader
    {
    public:
        AnyHitShader(const ShaderCodePointer& _code);

        ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(AnyHitShader, Shader);
    };

    EGO_POINTER(AnyHitShader);

    class IntersectionShader : public Shader
    {
    public:
        IntersectionShader(const ShaderCodePointer& _code);

        ShaderStage getShaderType() const override;

        EGO_GRAPHIC_RESOURCE(IntersectionShader, Shader);
    };

    EGO_POINTER(IntersectionShader);
} // namespace ego::gpu
