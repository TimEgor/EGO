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
        GraphicColorWriteMaskAll =
            GraphicColorWriteMaskRed |
            GraphicColorWriteMaskGreen |
            GraphicColorWriteMaskBlue |
            GraphicColorWriteMaskAlpha
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
    };

    struct DepthStencilStateDesc final
    {
        bool m_depthTestEnable = true;
        bool m_depthWrite = true;
        CompareOperation m_depthCompareOperation = CompareOperation::LessEqual;
        bool m_stencilEnable = false;
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
    };

    struct BlendStateDesc final
    {
        bool m_alphaToCoverageEnable = false;
        std::vector<RenderTargetBlendDesc> m_renderTargets;
    };

    struct GraphicPipelineDesc final
    {
        BindingLayoutReference m_bindingLayout = nullptr;

        VertexShaderReference m_vertexShader = nullptr;
        PixelShaderReference m_pixelShader = nullptr;

        InputLayoutDesc m_inputLayoutDesc;

        RasterizationStateDesc m_rasterizationStateDesc;
        DepthStencilStateDesc m_depthStencilStateDesc;
        BlendStateDesc m_blendStateDesc;
        int m_multisampleCount = 1;

        PrimitiveTopology m_topology = PrimitiveTopology::Undefined;

        std::vector<GraphicResourceFormat> m_colorFormats;
        GraphicResourceFormat m_depthFormat = GraphicResourceFormat::Undefined;
    };

    struct ComputePipelineDesc final
    {
        BindingLayoutReference m_bindingLayout = nullptr;
        ComputeShaderReference m_computeShader = nullptr;
    };

    enum class PipelineType
    {
        Graphic,
        Compute
    };

    class Pipeline : public GraphicResource
    {
    public:
        Pipeline() = default;

        virtual PipelineType getPipelineType() const = 0;

        EGO_GRAPHIC_RESOURCE(Pipeline, GraphicResource);
    };

    EGO_REFERENCE(Pipeline);

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

    EGO_REFERENCE(GraphicPipeline);

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

    EGO_REFERENCE(ComputePipeline);
}
