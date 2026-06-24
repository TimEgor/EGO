#include "RayTracingGeometryCache.h"

#include <algorithm>
#include <cstddef>
#include <limits>

#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"

void ego::render::RayTracingGeometryCache::beginFrame()
{
    ++m_frameIndex;
}

ego::render::RenderGeometryAccelerationStructure ego::render::RayTracingGeometryCache::requestGeometry(const MeshHandler& _mesh, ego::GraphicDevice& _graphicDevice)
{
    if (!_mesh)
    {
        return nullptr;
    }

    const size_t entryIndex = findEntryIndex(_mesh);
    if (entryIndex != std::numeric_limits<size_t>::max())
    {
        Entry& entry = m_entries[entryIndex];
        entry.m_lastUsedFrame = m_frameIndex;
        return entry.m_geometry;
    }

    gpu::MeshAccelerationStructureBuildDesc buildDesc;
    if (!FillGeometryDesc(*_mesh, buildDesc))
    {
        return nullptr;
    }

    const gpu::GpuGeometryAccelerationStructureTicket geometry = _graphicDevice.buildGeometryAccelerationStructure(buildDesc);
    if (!geometry)
    {
        return nullptr;
    }

    Entry entry;
    entry.m_mesh = _mesh;
    entry.m_geometry = geometry.m_resource;
    entry.m_lastUsedFrame = m_frameIndex;
    m_entries.push_back(entry);

    return entry.m_geometry;
}

void ego::render::RayTracingGeometryCache::removeUnused()
{
    m_entries.erase(
        std::remove_if(
            m_entries.begin(),
            m_entries.end(),
            [this](const Entry& _entry)
            {
                return _entry.m_lastUsedFrame != m_frameIndex;
            }),
        m_entries.end());
}

void ego::render::RayTracingGeometryCache::clear()
{
    m_entries.clear();
    m_frameIndex = 0;
}

bool ego::render::RayTracingGeometryCache::FillGeometryDesc(const Mesh& _mesh, gpu::MeshAccelerationStructureBuildDesc& _desc)
{
    const Mesh::VertexBufferBinding& vertexBuffer = _mesh.getVertexBuffer();
    if (!vertexBuffer.m_buffer || vertexBuffer.m_stride == 0 || _mesh.getVertexCount() < 3)
    {
        return false;
    }

    const Mesh::IndexBufferBinding& indexBuffer = _mesh.getIndexBuffer();
    const bool hasIndexBuffer = indexBuffer.m_buffer && _mesh.getIndexCount() >= 3;

    gpu::GeometryAccelerationStructureDesc geometryDesc;
    geometryDesc.m_vertexBuffer = vertexBuffer.m_buffer.getObject();
    geometryDesc.m_vertexOffset = vertexBuffer.m_offset;
    geometryDesc.m_vertexStride = vertexBuffer.m_stride;
    geometryDesc.m_vertexCount = _mesh.getVertexCount();
    geometryDesc.m_vertexFormat = gpu::GraphicResourceFormat::R32G32B32SFloat;
    geometryDesc.m_indexBuffer = hasIndexBuffer ? indexBuffer.m_buffer.getObject() : nullptr;
    geometryDesc.m_indexOffset = hasIndexBuffer ? indexBuffer.m_offset : 0;
    geometryDesc.m_indexCount = hasIndexBuffer ? _mesh.getIndexCount() : 0;
    geometryDesc.m_indexFormat = hasIndexBuffer ? indexBuffer.m_format : gpu::GraphicResourceFormat::Undefined;

    _desc.m_geometries.push_back(geometryDesc);
    return true;
}

size_t ego::render::RayTracingGeometryCache::findEntryIndex(const MeshHandler& _mesh) const
{
    const MeshReference mesh = _mesh.getObject();
    for (size_t entryIndex = 0; entryIndex < m_entries.size(); ++entryIndex)
    {
        if (m_entries[entryIndex].m_mesh.getObject() == mesh)
        {
            return entryIndex;
        }
    }

    return std::numeric_limits<size_t>::max();
}
