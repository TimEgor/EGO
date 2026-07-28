#pragma once

#include <cstdint>
#include <vector>

#include "BindingLayout.h"
#include "Format.h"
#include "GraphicResource.h"
#include "InputLayout.h"
#include "Sampler.h"
#include "Shader.h"

namespace ego::gpu
{
    enum class PrimitiveTopology
    {
        Undefined,

        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip
    };

    enum class RasterizationFillMode
    {
        Solid,
        Wireframe
    };

    enum class RasterizationCullMode
    {
        None,
        BackFace,
        FrontFace
    };

    enum class BlendFactor
    {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha
    };

    enum class BlendOperation
    {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    enum GraphicColorWriteMask : uint8_t
    {
        GraphicColorWriteMaskNone = 0,
        GraphicColorWriteMaskRed = 1 << 0,
        GraphicColorWriteMaskGreen = 1 << 1,
        GraphicColorWriteMaskBlue = 1 << 2,
        GraphicColorWriteMaskAlpha = 1 << 3,
        GraphicColorWriteMaskAll = GraphicColorWriteMaskRed | GraphicColorWriteMaskGreen | GraphicColorWriteMaskBlue | GraphicColorWriteMaskAlpha
    };

    struct RasterizationStateDesc final
    {
        RasterizationFillMode m_fillMode = RasterizationFillMode::Solid;
        RasterizationCullMode m_cullMode = RasterizationCullMode::BackFace;
        bool m_frontCounterClockwise = false;
        bool m_depthClip = true;
        bool m_depthBiasEnable = false;
        int32_t m_depthBias = 0;
        float m_depthBiasClamp = 0.0f;
        float m_depthBiasSlopeScale = 0.0f;

        bool operator==(const RasterizationStateDesc& _other) const;
        bool operator!=(const RasterizationStateDesc& _other) const;
    };

    struct DepthStencilStateDesc final
    {
        bool m_depthTestEnable = true;
        bool m_depthWrite = true;
        CompareOperation m_depthCompareOperation = CompareOperation::LessEqual;
        bool m_stencilEnable = false;

        bool operator==(const DepthStencilStateDesc& _other) const;
        bool operator!=(const DepthStencilStateDesc& _other) const;
    };

    struct RenderTargetBlendDesc final
    {
        bool m_blendEnable = false;
        BlendFactor m_srcColorFactor = BlendFactor::One;
        BlendFactor m_dstColorFactor = BlendFactor::Zero;
        BlendOperation m_colorOperation = BlendOperation::Add;
        BlendFactor m_srcAlphaFactor = BlendFactor::One;
        BlendFactor m_dstAlphaFactor = BlendFactor::Zero;
        BlendOperation m_alphaOperation = BlendOperation::Add;
        uint8_t m_colorWriteMask = GraphicColorWriteMaskAll;

        bool operator==(const RenderTargetBlendDesc& _other) const;
        bool operator!=(const RenderTargetBlendDesc& _other) const;
    };

    struct BlendStateDesc final
    {
        bool m_alphaToCoverageEnable = false;
        std::vector<RenderTargetBlendDesc> m_renderTargets;

        bool operator==(const BlendStateDesc& _other) const;
        bool operator!=(const BlendStateDesc& _other) const;
    };

    struct GraphicPipelineDesc final
    {
        BindingLayoutPointer m_bindingLayout = nullptr;

        VertexShaderPointer m_vertexShader = nullptr;
        PixelShaderPointer m_pixelShader = nullptr;

        InputLayoutDesc m_inputLayoutDesc;

        RasterizationStateDesc m_rasterizationStateDesc;
        DepthStencilStateDesc m_depthStencilStateDesc;
        BlendStateDesc m_blendStateDesc;
        int m_multisampleCount = 1;

        PrimitiveTopology m_topology = PrimitiveTopology::Undefined;

        std::vector<GraphicResourceFormat> m_colorFormats;
        GraphicResourceFormat m_depthFormat = GraphicResourceFormat::Undefined;

        bool operator==(const GraphicPipelineDesc& _other) const;
        bool operator!=(const GraphicPipelineDesc& _other) const;
    };

    struct ComputePipelineDesc final
    {
        BindingLayoutPointer m_bindingLayout = nullptr;
        ComputeShaderPointer m_computeShader = nullptr;

        bool operator==(const ComputePipelineDesc& _other) const;
        bool operator!=(const ComputePipelineDesc& _other) const;
    };

    enum class RayTracingHitGroupType
    {
        Triangles,
        ProceduralPrimitive
    };

    struct RayTracingHitGroupDesc final
    {
        RayTracingHitGroupType m_type = RayTracingHitGroupType::Triangles;
        ClosestHitShaderPointer m_closestHitShader = nullptr;
        AnyHitShaderPointer m_anyHitShader = nullptr;
        IntersectionShaderPointer m_intersectionShader = nullptr;

        bool operator==(const RayTracingHitGroupDesc& _other) const;
        bool operator!=(const RayTracingHitGroupDesc& _other) const;
    };

    struct RayTracingPipelineDesc final
    {
        BindingLayoutPointer m_bindingLayout = nullptr;
        RayGenerationShaderPointer m_rayGenerationShader = nullptr;
        MissShaderPointer m_missShader = nullptr;
        std::vector<RayTracingHitGroupDesc> m_hitGroups;
        uint32_t m_maxPayloadSize = 0;
        uint32_t m_maxAttributeSize = 0;
        uint32_t m_maxRecursionDepth = 1;

        bool operator==(const RayTracingPipelineDesc& _other) const;
        bool operator!=(const RayTracingPipelineDesc& _other) const;
    };

    enum class PipelineType
    {
        Graphic,
        Compute,
        RayTracing
    };

    class Pipeline : public GraphicResource
    {
    public:
        Pipeline() = default;

        virtual PipelineType getPipelineType() const = 0;

        EGO_GRAPHIC_RESOURCE(Pipeline, GraphicResource);
    };

    EGO_INTRUSIVE_POINTER(Pipeline);

    class GraphicPipeline : public Pipeline
    {
    public:
        GraphicPipeline(const GraphicPipelineDesc& _desc);

        const GraphicPipelineDesc& getDesc() const;

        PipelineType getPipelineType() const override;

        EGO_GRAPHIC_RESOURCE(GraphicPipeline, Pipeline);

    private:
        GraphicPipelineDesc m_desc;
    };

    EGO_INTRUSIVE_POINTER(GraphicPipeline);

    class ComputePipeline : public Pipeline
    {
    public:
        ComputePipeline(const ComputePipelineDesc& _desc);

        const ComputePipelineDesc& getDesc() const;

        PipelineType getPipelineType() const override;

        EGO_GRAPHIC_RESOURCE(ComputePipeline, Pipeline);

    private:
        ComputePipelineDesc m_desc;
    };

    EGO_INTRUSIVE_POINTER(ComputePipeline);

    class RayTracingPipeline : public Pipeline
    {
    public:
        RayTracingPipeline(const RayTracingPipelineDesc& _desc);

        const RayTracingPipelineDesc& getDesc() const;

        PipelineType getPipelineType() const override;

        EGO_GRAPHIC_RESOURCE(RayTracingPipeline, Pipeline);

    private:
        RayTracingPipelineDesc m_desc;
    };

    EGO_INTRUSIVE_POINTER(RayTracingPipeline);
} // namespace ego::gpu
