#pragma once

#include <cstdint>

#include "EgoCore/RTTI/RTTI.h"

#include "GpuTask.h"
#include "GraphicObject.h"

namespace ego::gpu
{
    enum GraphicResourceUsage
    {
        GraphicResourceUsageUndefined = 0,

        GraphicResourceUsageTransferSrc = 1 << 0,
        GraphicResourceUsageTransferDst = 1 << 1,
        GraphicResourceUsageShaderResource = 1 << 2,
        GraphicResourceUsageAllowUnorderedAccess = 1 << 3,
        GraphicResourceUsageRayTracingAccelerationStructure = 1 << 4,

        GraphicResourceMaxUsage = GraphicResourceUsageRayTracingAccelerationStructure
    };

    enum CommonGraphicResourceAccess
    {
        GraphicResourceAccessUndefined = 0,

        GraphicResourceAccessCpuRead = 1 << 0,
        GraphicResourceAccessCpuWrite = 1 << 1,
        GraphicResourceAccessGpuRead = 1 << 2,
        GraphicResourceAccessGpuWrite = 1 << 3
    };

    enum class GraphicResourceState
    {
        Undefined,

        Common,
        CopySrc,
        CopyDst,
        VertexBuffer,
        IndexBuffer,
        ConstantBuffer,
        ShaderRead,
        UnorderedAccess,
        RenderTarget,
        DepthWrite,
        DepthRead,
        IndirectBuffer,
        RayTracingAccelerationStructure,
        Present
    };

    struct InitialGraphicResourceData final
    {
        const void* m_data = nullptr;
        uint32_t m_dataSize = 0;
        uint32_t m_rowPitch = 0;
        uint32_t m_slicePitch = 0;

        InitialGraphicResourceData() = default;

        InitialGraphicResourceData(const void* _data, uint32_t _dataSize, uint32_t _rowPitch = 0, uint32_t _slicePitch = 0);

        bool isValid() const;
    };

    struct GraphicResourceDesc
    {
        GraphicResourceUsage m_usage = GraphicResourceUsageUndefined;
        CommonGraphicResourceAccess m_access = GraphicResourceAccessUndefined;
    };

    using GraphicResourceType = rtti::TypeMetaInfoID;

    class GraphicResource : public GraphicObject
    {
    public:
        GraphicResource() = default;

        virtual GraphicResourceType getType() const = 0;
        virtual const char* getTypeName() const = 0;

        bool isGpuReady() const;
        void waitGpuReady() const;
        const GpuTaskPointer& getLastWriteTask() const;
        void setLastWriteTask(const GpuTaskPointer& _task);
        GraphicResourceState getState() const;
        void setState(GraphicResourceState _state);

        EGO_RTTI_VIRTUAL_BASE(GraphicResource);

    private:
        GpuTaskPointer m_lastWriteTask = nullptr;
        GraphicResourceState m_state = GraphicResourceState::Common;
    };

    EGO_POINTER(GraphicResource);
} // namespace ego::gpu

#define EGO_GRAPHIC_RESOURCE_TYPE_INFO()                                                                                                                       \
    static const char* GetGraphicResourceTypeName()                                                                                                            \
    {                                                                                                                                                          \
        return GetMetaInfoTypeName();                                                                                                                          \
    }                                                                                                                                                          \
                                                                                                                                                               \
    static ego::gpu::GraphicResourceType GetGraphicResourceType()                                                                                              \
    {                                                                                                                                                          \
        return GetMetaInfoID();                                                                                                                                \
    }

#define EGO_GRAPHIC_RESOURCE(_TYPE, ...)                                                                                                                       \
    EGO_RTTI_VIRTUAL(_TYPE, __VA_ARGS__);                                                                                                                      \
    EGO_GRAPHIC_RESOURCE_TYPE_INFO();                                                                                                                          \
                                                                                                                                                               \
    virtual const char* getTypeName() const override                                                                                                           \
    {                                                                                                                                                          \
        return GetGraphicResourceTypeName();                                                                                                                   \
    }                                                                                                                                                          \
                                                                                                                                                               \
    virtual ego::gpu::GraphicResourceType getType() const override                                                                                             \
    {                                                                                                                                                          \
        return GetGraphicResourceType();                                                                                                                       \
    }

#define EGO_GRAPHIC_RESOURCE_TYPE(_RESOURCE) (_RESOURCE::GetGraphicResourceType())
#define EGO_GRAPHIC_RESOURCE_TYPE_NAME(_RESOURCE) (_RESOURCE::GetGraphicResourceTypeName())
