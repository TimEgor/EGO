#pragma once

#include "EgoGraphicHardware/GraphicDevice.h"

#include "EgoEngine/Graphic/Render/DebugDrawData.h"
#include "EgoEngine/Graphic/Render/RenderObject.h"

#include "DebugRenderElementData.h"

namespace ego::render
{
    class RenderPipelineStateCache;

    class DefaultRenderDebugDraw final
    {
    public:
        struct ShaderInitData final
        {
            RenderVertexShader m_vertexShader = nullptr;
            RenderPixelShader m_pixelShader = nullptr;
        };

        struct InitData final
        {
            ShaderInitData m_point;
            ShaderInitData m_line;
        };

        bool init(const RenderBindingLayout& _bindingLayout, gpu::GraphicResourceFormat _renderTargetFormat, const InitData& _initData);
        void release();
        void clearResources();

        bool prepare(float _deltaTime);
        void render(
            GraphicDevice& _graphicDevice,
            RenderPipelineStateCache& _pipelineStateCache,
            const RenderGraphicCommandList& _commandList,
            const RenderBufferView& _cameraShaderDataView);

        void drawPoint(const DebugDrawPointData& _point);
        void drawLine(const DebugDrawLineData& _line);

    private:
        void clearCommands();
        bool preparePointData(float _deltaTime);
        void appendLine(const DebugLineRenderData::VertexData& _start, const DebugLineRenderData::VertexData& _end);

        bool prepareLineData(float _deltaTime);

        RenderGraphicPipeline getOrCreateDebugDrawPipeline(
            GraphicDevice& _graphicDevice,
            RenderPipelineStateCache& _pipelineStateCache,
            const RasterizationMaterialRenderPassInfoReference& _materialInfo,
            const gpu::InputLayoutDesc& _inputLayoutDesc,
            gpu::PrimitiveTopology _topology) const;

        void renderPoints(
            GraphicDevice& _graphicDevice,
            RenderPipelineStateCache& _pipelineStateCache,
            const RenderGraphicCommandList& _commandList,
            const RenderBufferView& _cameraShaderDataView);
        void renderLines(
            GraphicDevice& _graphicDevice,
            RenderPipelineStateCache& _pipelineStateCache,
            const RenderGraphicCommandList& _commandList,
            const RenderBufferView& _cameraShaderDataView);

        static gpu::InputLayoutDesc CreatePointInputLayout();
        static gpu::InputLayoutDesc CreateLineInputLayout();
        static RasterizationMaterialRenderPassInfoReference CreateDebugDrawMaterialInfo(const RenderVertexShader& _vertexShader, const RenderPixelShader& _pixelShader);

        RenderBindingLayout m_bindingLayout = nullptr;
        gpu::GraphicResourceFormat m_renderTargetFormat = gpu::GraphicResourceFormat::Undefined;
        DebugPointRenderData m_pointRenderData;
        DebugLineRenderData m_lineRenderData;
    };
} // namespace ego::render
