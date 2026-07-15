#pragma once

#include <vector>

#include "EgoCore/Math/Matrix.h"

#include "Buffer.h"
#include "GraphicResource.h"
#include "ResourceView.h"

namespace ego::gpu
{
    class GeometryAccelerationStructure;
    EGO_REFERENCE(GeometryAccelerationStructure);

    class InstanceAccelerationStructure;
    EGO_REFERENCE(InstanceAccelerationStructure);

    struct GeometryAccelerationStructureDesc final
    {
        BufferReference m_vertexBuffer = nullptr;
        uint64_t m_vertexOffset = 0;
        uint32_t m_vertexStride = 0;
        uint32_t m_vertexCount = 0;
        GraphicResourceFormat m_vertexFormat = GraphicResourceFormat::R32G32B32SFloat;

        BufferReference m_indexBuffer = nullptr;
        uint64_t m_indexOffset = 0;
        uint32_t m_indexCount = 0;
        GraphicResourceFormat m_indexFormat = GraphicResourceFormat::Undefined;
    };

    struct MeshAccelerationStructureBuildDesc final
    {
        std::vector<GeometryAccelerationStructureDesc> m_geometries;
    };

    struct InstanceGeometryAccelerationStructureBuildDesc final
    {
        GeometryAccelerationStructureReference m_geometry = nullptr;
        FloatMatrix4x4 m_transform = FloatMatrix4x4Identity;
        uint32_t m_instanceId = 0;
        uint32_t m_instanceMask = 0xff;
        uint32_t m_hitGroupIndex = 0;
    };

    struct InstanceAccelerationStructureBuildDesc final
    {
        std::vector<InstanceGeometryAccelerationStructureBuildDesc> m_instances;
    };

    class AccelerationStructure : public GraphicResource
    {
    public:
        virtual const BufferReference& getBuffer() const = 0;

        EGO_GRAPHIC_RESOURCE(AccelerationStructure, GraphicResource);
    };

    EGO_REFERENCE(AccelerationStructure);

    class GeometryAccelerationStructure : public AccelerationStructure
    {
    public:
        GeometryAccelerationStructure() = default;

        EGO_GRAPHIC_RESOURCE(GeometryAccelerationStructure, AccelerationStructure);
    };

    class InstanceAccelerationStructure : public AccelerationStructure
    {
    public:
        InstanceAccelerationStructure() = default;

        EGO_GRAPHIC_RESOURCE(InstanceAccelerationStructure, AccelerationStructure);
    };

    class AccelerationStructureView : public ResourceView
    {
    public:
        AccelerationStructureView(const InstanceAccelerationStructureReference& _accelerationStructure);

        GraphicResourceViewType getViewType() const override;
    };

    EGO_REFERENCE(AccelerationStructureView);
} // namespace ego::gpu
