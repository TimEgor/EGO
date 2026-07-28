#pragma once

#include <cstdint>

#include "EgoEngine/Graphic/SceneRender/RenderGpuObject.h"

namespace ego
{
    class GraphicDevice;
}

namespace ego::render
{
    struct MeshRawData final
    {
        const void* m_vertexData = nullptr;
        uint32_t m_vertexDataSize = 0;
        uint32_t m_vertexStride = 0;
        uint32_t m_vertexOffset = 0;
        uint32_t m_vertexCount = 0;

        const void* m_indexData = nullptr;
        uint32_t m_indexDataSize = 0;
        gpu::GraphicResourceFormat m_indexFormat = gpu::GraphicResourceFormat::Undefined;
        uint32_t m_indexOffset = 0;
        uint32_t m_indexCount = 0;
    };

    class Mesh final : public MTCountable
    {
    public:
        struct VertexBufferBinding final
        {
            RenderBuffer m_buffer = nullptr;
            uint32_t m_stride = 0;
            uint32_t m_offset = 0;
        };

        struct IndexBufferBinding final
        {
            RenderBuffer m_buffer = nullptr;
            gpu::GraphicResourceFormat m_format = gpu::GraphicResourceFormat::Undefined;
            uint32_t m_offset = 0;
        };

        Mesh(const VertexBufferBinding& _vertexBuffer, uint32_t _vertexCount, const IndexBufferBinding& _indexBuffer = IndexBufferBinding(), uint32_t _indexCount = 0);

        const VertexBufferBinding& getVertexBuffer() const;
        uint32_t getVertexCount() const;

        const IndexBufferBinding& getIndexBuffer() const;
        uint32_t getIndexCount() const;

    private:
        const VertexBufferBinding m_vertexBuffer;
        const IndexBufferBinding m_indexBuffer;
        const uint32_t m_vertexCount;
        const uint32_t m_indexCount;
    };

    EGO_INTRUSIVE_POINTER(Mesh);
    EGO_NAMED_HANDLER(MeshPointer, Mesh);

    MeshHandler CreateMeshFromRawData(ego::GraphicDevice& _graphicDevice, const MeshRawData& _rawData);
} // namespace ego::render
