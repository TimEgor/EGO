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

const ego::MeshReference& ego::MeshResource::getMesh() const
{
    return m_mesh;
}

void ego::MeshResource::setMesh(const MeshReference& _mesh)
{
    m_mesh = _mesh;
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

    MeshRawData rawData;
    rawData.m_vertexData = vertices.data();
    rawData.m_vertexDataSize = static_cast<uint32_t>(vertices.size() * sizeof(MeshVertex));
    rawData.m_vertexStride = sizeof(MeshVertex);
    rawData.m_vertexCount = static_cast<uint32_t>(vertices.size());

    const MeshReference mesh = CreateMeshFromRawData(engine::GetEngine().getGraphicDevice(), rawData);
    EGO_CHECK_RETURN_FALSE(mesh);

    setMesh(mesh);

    return true;
}

void ego::MeshResource::onUnload()
{
    m_mesh = nullptr;
}
