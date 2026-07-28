#include "AccelerationStructure.h"

ego::gpu::AccelerationStructureView::AccelerationStructureView(const InstanceAccelerationStructurePointer& _accelerationStructure)
    : ResourceView(_accelerationStructure)
{
}

ego::gpu::GraphicResourceViewType ego::gpu::AccelerationStructureView::getViewType() const
{
    return GraphicResourceViewType::ShaderResource;
}
