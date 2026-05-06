#include "GraphicResource.h"

ego::gpu::InitialGraphicResourceData::InitialGraphicResourceData(
    const void* _data,
    uint32_t _dataSize,
    uint32_t _rowPitch,
    uint32_t _slicePitch
)
    : m_data(_data),
      m_dataSize(_dataSize),
      m_rowPitch(_rowPitch),
      m_slicePitch(_slicePitch)
{}

bool ego::gpu::InitialGraphicResourceData::isValid() const
{
    return m_data != nullptr && m_dataSize != 0;
}
