#pragma once

#include "EgoEngine/Graphic/Render/DebugDrawData.h"
#include "EgoEngine/Graphic/Render/RenderObject.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"

#include "DebugRenderElementData.h"

namespace ego::render
{
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

        bool init(GraphicDevice& _graphicDevice, const RenderBindingLayout& _bindingLayout, gpu::GraphicResourceFormat _renderTargetFormat, const InitData& _initData);
        void release();
        void clearResources();

        bool prepare();
        void render(const RenderGraphicCommandList& _commandList, const RenderBufferView& _cameraShaderDataView);

        void drawPoint(const DebugDrawPointData& _point);
        void drawLine(const DebugDrawLineData& _line);

    private:
        void clearCommands();
        bool preparePointData();
        void appendLine(const DebugLineRenderData::VertexData& _start, const DebugLineRenderData::VertexData& _end);

        bool prepareLineData();

        void renderPoints(const RenderGraphicCommandList& _commandList, const RenderBufferView& _cameraShaderDataView);
        void renderLines(const RenderGraphicCommandList& _commandList, const RenderBufferView& _cameraShaderDataView);

        static gpu::InputLayoutDesc CreatePointInputLayout();
        static gpu::InputLayoutDesc CreateLineInputLayout();
        static RenderGraphicPipeline CreateDebugDrawPipeline(
            GraphicDevice& _graphicDevice,
            const RenderBindingLayout& _bindingLayout,
            const RenderVertexShader& _vertexShader,
            const RenderPixelShader& _pixelShader,
            const gpu::InputLayoutDesc& _inputLayoutDesc,
            gpu::GraphicResourceFormat _renderTargetFormat,
            gpu::PrimitiveTopology _topology);

        DebugPointRenderData m_pointRenderData;
        DebugLineRenderData m_lineRenderData;
    };
} // namespace ego::render
