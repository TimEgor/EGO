#include "MeshResource.h"

#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoMath/Vector.h"

#include "EgoEngine/Engine.h"

#include <vector>

namespace
{
    struct MeshVertex final
    {
        ego::FloatVector3 m_position = ego::FloatVector3Zero;
        ego::FloatVector4 m_color = ego::FloatVector4One;
    };

    float ReadFloat(const ego::XmlNode& _node, const char* _name, float _defaultValue = 0.0f)
    {
        const ego::XmlNode child = _node.getChild(_name);
        return child ? child.getValue().get<float>() : _defaultValue;
    }

    MeshVertex ReadVertex(const ego::XmlNode& _node)
    {
        MeshVertex vertex;

        const ego::XmlNode position = _node.getChild("Position");
        vertex.m_position.m_x = ReadFloat(position, "X");
        vertex.m_position.m_y = ReadFloat(position, "Y");
        vertex.m_position.m_z = ReadFloat(position, "Z");

        const ego::XmlNode color = _node.getChild("Color");
        vertex.m_color.m_x = ReadFloat(color, "R", 1.0f);
        vertex.m_color.m_y = ReadFloat(color, "G", 1.0f);
        vertex.m_color.m_z = ReadFloat(color, "B", 1.0f);
        vertex.m_color.m_w = ReadFloat(color, "A", 1.0f);

        return vertex;
    }
}

const ego::VertexBufferBinding& ego::MeshResource::getVertexBuffer() const
{
    return m_vertexBuffer;
}

void ego::MeshResource::setVertexBuffer(
    const gpu::BufferPointer& _buffer,
    uint32_t _stride,
    uint32_t _offset
)
{
    m_vertexBuffer.m_buffer = _buffer;
    m_vertexBuffer.m_stride = _stride;
    m_vertexBuffer.m_offset = _offset;
}

const ego::gpu::BufferPointer& ego::MeshResource::getIndexBuffer() const
{
    return m_indexBuffer;
}

ego::gpu::GraphicResourceFormat ego::MeshResource::getIndexFormat() const
{
    return m_indexFormat;
}

uint32_t ego::MeshResource::getIndexOffset() const
{
    return m_indexOffset;
}

void ego::MeshResource::setIndexBuffer(
    const gpu::BufferPointer& _buffer,
    gpu::GraphicResourceFormat _format,
    uint32_t _offset
)
{
    m_indexBuffer = _buffer;
    m_indexFormat = _format;
    m_indexOffset = _offset;
}

uint32_t ego::MeshResource::getVertexCount() const
{
    return m_vertexCount;
}

void ego::MeshResource::setVertexCount(uint32_t _count)
{
    m_vertexCount = _count;
}

uint32_t ego::MeshResource::getIndexCount() const
{
    return m_indexCount;
}

void ego::MeshResource::setIndexCount(uint32_t _count)
{
    m_indexCount = _count;
}

bool ego::MeshResource::onLoad(FileContent&& _content, ResourceLoadingContext&)
{
    XmlDocument document;
    EGO_CHECK_RETURN_FALSE(!_content.empty() && document.loadFromBuffer(_content.data(), _content.size()));

    const XmlNode meshNode = document.getRootNode().getChild("Mesh");
    const XmlNode verticesNode = meshNode.getChild("Vertices");
    EGO_CHECK_RETURN_FALSE(verticesNode);

    std::vector<MeshVertex> vertices;
    for (const XmlNode& vertexNode : verticesNode.getChildren("Vertex"))
    {
        vertices.push_back(ReadVertex(vertexNode));
    }

    EGO_CHECK_RETURN_FALSE(!vertices.empty());

    gpu::BufferDesc vertexBufferDesc;
    vertexBufferDesc.m_usage = static_cast<gpu::GraphicResourceUsage>(gpu::GpuBufferUsageVertexBuffer);
    vertexBufferDesc.m_access = gpu::GraphicResourceAccessCpuWrite;
    vertexBufferDesc.m_size = static_cast<uint32_t>(vertices.size() * sizeof(MeshVertex));
    vertexBufferDesc.m_stride = sizeof(MeshVertex);

    const gpu::InitialGraphicResourceData vertexData(vertices.data(), vertexBufferDesc.m_size);
    gpu::BufferPointer vertexBuffer = engine::GetEngine().getGraphicDevice().createBuffer(vertexBufferDesc, vertexData);
    EGO_CHECK_RETURN_FALSE(vertexBuffer);

    setVertexBuffer(vertexBuffer, sizeof(MeshVertex));
    setVertexCount(static_cast<uint32_t>(vertices.size()));

    return true;
}

void ego::MeshResource::onUnload()
{
    m_vertexBuffer = VertexBufferBinding();
    m_indexBuffer = nullptr;
    m_indexFormat = gpu::GraphicResourceFormat::Undefined;
    m_indexOffset = 0;
    m_vertexCount = 0;
    m_indexCount = 0;
}
