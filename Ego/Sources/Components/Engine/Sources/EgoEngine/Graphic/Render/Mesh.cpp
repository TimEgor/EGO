#include "Mesh.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"

namespace
{
    uint32_t GetIndexFormatSize(ego::gpu::GraphicResourceFormat _format)
    {
        switch (_format)
        {
        case ego::gpu::GraphicResourceFormat::R16UInt:
            return sizeof(uint16_t);
        case ego::gpu::GraphicResourceFormat::R32UInt:
            return sizeof(uint32_t);
        default:
            return 0;
        }
    }

    uint32_t ResolveElementCount(uint32_t _explicitCount, uint32_t _dataSize, uint32_t _stride)
    {
        if (_explicitCount != 0)
        {
            return _explicitCount;
        }

        return _stride != 0 ? _dataSize / _stride : 0;
    }
}

ego::Mesh::Mesh(
    const VertexBufferBinding& _vertexBuffer,
    uint32_t _vertexCount,
    const IndexBufferBinding& _indexBuffer,
    uint32_t _indexCount
)
    : m_vertexBuffer(_vertexBuffer),
      m_indexBuffer(_indexBuffer),
      m_vertexCount(_vertexCount),
      m_indexCount(_indexCount)
{}

const ego::Mesh::VertexBufferBinding& ego::Mesh::getVertexBuffer() const
{
    return m_vertexBuffer;
}

const ego::Mesh::IndexBufferBinding& ego::Mesh::getIndexBuffer() const
{
    return m_indexBuffer;
}

uint32_t ego::Mesh::getVertexCount() const
{
    return m_vertexCount;
}

uint32_t ego::Mesh::getIndexCount() const
{
    return m_indexCount;
}

ego::MeshReference ego::CreateMeshFromRawData(GraphicDevice& _graphicDevice, const MeshRawData& _rawData)
{
    if (!_rawData.m_vertexData || _rawData.m_vertexDataSize == 0 || _rawData.m_vertexStride == 0)
    {
        return nullptr;
    }

    const uint32_t vertexCount = ResolveElementCount(
        _rawData.m_vertexCount,
        _rawData.m_vertexDataSize,
        _rawData.m_vertexStride
    );
    if (vertexCount == 0)
    {
        return nullptr;
    }

    gpu::BufferDesc vertexBufferDesc;
    vertexBufferDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageVertexBuffer);
    vertexBufferDesc.m_access = gpu::GraphicResourceAccessCpuWrite;
    vertexBufferDesc.m_size = _rawData.m_vertexDataSize;
    vertexBufferDesc.m_stride = _rawData.m_vertexStride;

    const gpu::InitialGraphicResourceData vertexData(_rawData.m_vertexData, _rawData.m_vertexDataSize);
    const gpu::BufferReference vertexBuffer = _graphicDevice.createBuffer(vertexBufferDesc, vertexData);
    if (!vertexBuffer)
    {
        return nullptr;
    }

    gpu::BufferReference indexBuffer = nullptr;
    uint32_t indexCount = 0;
    if (_rawData.m_indexData && _rawData.m_indexDataSize != 0)
    {
        const uint32_t indexStride = GetIndexFormatSize(_rawData.m_indexFormat);
        if (indexStride == 0)
        {
            return nullptr;
        }

        indexCount = ResolveElementCount(_rawData.m_indexCount, _rawData.m_indexDataSize, indexStride);
        if (indexCount == 0)
        {
            return nullptr;
        }

        gpu::BufferDesc indexBufferDesc;
        indexBufferDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageIndexBuffer);
        indexBufferDesc.m_access = gpu::GraphicResourceAccessCpuWrite;
        indexBufferDesc.m_size = _rawData.m_indexDataSize;
        indexBufferDesc.m_stride = indexStride;

        const gpu::InitialGraphicResourceData indexData(_rawData.m_indexData, _rawData.m_indexDataSize);
        indexBuffer = _graphicDevice.createBuffer(indexBufferDesc, indexData);
        if (!indexBuffer)
        {
            return nullptr;
        }
    }

    Mesh::VertexBufferBinding vertexBufferBinding;
    vertexBufferBinding.m_buffer = vertexBuffer;
    vertexBufferBinding.m_stride = _rawData.m_vertexStride;
    vertexBufferBinding.m_offset = _rawData.m_vertexOffset;

    Mesh::IndexBufferBinding indexBufferBinding;
    indexBufferBinding.m_buffer = indexBuffer;
    indexBufferBinding.m_format = indexBuffer ? _rawData.m_indexFormat : gpu::GraphicResourceFormat::Undefined;
    indexBufferBinding.m_offset = indexBuffer ? _rawData.m_indexOffset : 0;

    return MeshReference(new Mesh(
        vertexBufferBinding,
        vertexCount,
        indexBufferBinding,
        indexCount
    ));
}

ego::MeshHandle ego::CreateMeshHandle(const MeshReference& _mesh)
{
    return MakeHandle<Mesh>(_mesh);
}
