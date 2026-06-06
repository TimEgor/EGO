#pragma once

#include <cstdint>
#include <vector>

#include "EgoCore/FileName/FileName.h"

#include "EgoMath/Vector.h"

#include "EgoEngine/Graphic/Render/DebugDrawData.h"
#include "EgoEngine/Graphic/Render/RenderObject.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"

namespace ego::render
{
    class DefaultRenderDebugDraw final
    {
    public:
        bool init(
            const FileName& _pluginRootPath,
            GraphicDevice& _graphicDevice,
            const RenderBindingLayout& _bindingLayout,
            gpu::GraphicResourceFormat _renderTargetFormat
        );
        void release();
        void clearResources();

        bool prepare();
        void render(
            const RenderGraphicCommandList& _commandList,
            const RenderBufferView& _cameraShaderDataView
        );

        void drawPoint(const DebugDrawPointData& _point);
        void drawLine(const DebugDrawLineData& _line);

    private:
        using VertexCollection = std::vector<DebugDrawPointData>;

        void clearCommands();
        void appendLineVertex(
            const FloatVector3& _position,
            const FloatVector4& _color
        );
        bool prepareBuffer(
            const VertexCollection& _vertices,
            RenderBuffer& _buffer,
            uint32_t& _vertexCapacity
        );
        void renderVertices(
            const RenderGraphicCommandList& _commandList,
            const RenderBufferView& _cameraShaderDataView,
            const RenderGraphicPipeline& _pipeline,
            const RenderBuffer& _vertexBuffer,
            uint32_t _vertexCount
        );

        RenderGraphicPipeline m_linePipeline = nullptr;
        RenderBuffer m_lineVertexBuffer = nullptr;
        VertexCollection m_lineVertices;
        uint32_t m_lineVertexCount = 0;
        uint32_t m_lineVertexCapacity = 0;
    };
}
