#pragma once

#include <vector>

#include "EgoCore/Math/Matrix.h"

#include "Buffer.h"
#include "GraphicResource.h"
#include "ResourceView.h"

namespace ego::gpu
{
    class GeometryAccelerationStructure;
    EGO_POINTER(GeometryAccelerationStructure);

    class InstanceAccelerationStructure;
    EGO_POINTER(InstanceAccelerationStructure);

    struct GeometryAccelerationStructureDesc final
    {
        BufferPointer m_vertexBuffer = nullptr;
        uint64_t m_vertexOffset = 0;
        uint32_t m_vertexStride = 0;
        uint32_t m_vertexCount = 0;
        GraphicResourceFormat m_vertexFormat = GraphicResourceFormat::R32G32B32SFloat;

        BufferPointer m_indexBuffer = nullptr;
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
        GeometryAccelerationStructurePointer m_geometry = nullptr;
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
        virtual const BufferPointer& getBuffer() const = 0;

        EGO_GRAPHIC_RESOURCE(AccelerationStructure, GraphicResource);
    };

    EGO_POINTER(AccelerationStructure);

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
        AccelerationStructureView(const InstanceAccelerationStructurePointer& _accelerationStructure);

        GraphicResourceViewType getViewType() const override;
    };

    EGO_POINTER(AccelerationStructureView);
} // namespace ego::gpu
