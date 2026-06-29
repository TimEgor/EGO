#include "InputLayout.h"

#include <cstring>

#include "EgoCore/Hash/Crc32.h"
#include "EgoCore/Hash/HashCombine.h"

namespace ego::gpu::input_layout_details
{
    bool AreStringsEqual(const char* _lhs, const char* _rhs)
    {
        return _lhs == _rhs || (_lhs && _rhs && std::strcmp(_lhs, _rhs) == 0);
    }
} // namespace ego::gpu::input_layout_details

bool ego::gpu::InputLayoutElementDesc::operator==(const InputLayoutElementDesc& _other) const
{
    return m_location == _other.m_location && m_index == _other.m_index && m_slot == _other.m_slot && m_offset == _other.m_offset &&
           m_componentsCount == _other.m_componentsCount && m_type == _other.m_type && input_layout_details::AreStringsEqual(m_semanticName, _other.m_semanticName);
}

bool ego::gpu::InputLayoutElementDesc::operator!=(const InputLayoutElementDesc& _other) const
{
    return !(*this == _other);
}

ego::gpu::InputLayoutElementHash ego::gpu::InputLayoutElementDesc::getHash() const
{
    InputLayoutElementHash result = 0;

    result = HashCombine(result, m_semanticName ? ego::Crc32(m_semanticName) : 0);
    result = HashCombine(result, m_location);
    result = HashCombine(result, m_index);
    result = HashCombine(result, m_slot);
    result = HashCombine(result, m_offset);
    result = HashCombine(result, m_componentsCount);
    result = HashCombine(result, static_cast<uint64_t>(m_type));

    return result;
}

bool ego::gpu::InputLayoutBindingDesc::operator==(const InputLayoutBindingDesc& _other) const
{
    return m_slot == _other.m_slot && m_stride == _other.m_stride && m_type == _other.m_type && m_instanceStepRate == _other.m_instanceStepRate;
}

bool ego::gpu::InputLayoutBindingDesc::operator!=(const InputLayoutBindingDesc& _other) const
{
    return !(*this == _other);
}

ego::gpu::InputLayoutBindingHash ego::gpu::InputLayoutBindingDesc::getHash() const
{
    InputLayoutBindingHash result = 0;

    result = HashCombine(result, m_slot);
    result = HashCombine(result, m_stride);
    result = HashCombine(result, static_cast<uint64_t>(m_type));
    result = HashCombine(result, m_instanceStepRate);

    return result;
}

bool ego::gpu::InputLayoutDesc::operator==(const InputLayoutDesc& _other) const
{
    return m_elements == _other.m_elements && m_bindings == _other.m_bindings;
}

bool ego::gpu::InputLayoutDesc::operator!=(const InputLayoutDesc& _other) const
{
    return !(*this == _other);
}

ego::gpu::InputLayoutHash ego::gpu::InputLayoutDesc::getHash() const
{
    InputLayoutHash result = 0;

    for (const auto& element : m_elements)
    {
        result = HashCombine(result, element.getHash());
    }

    for (const auto& binding : m_bindings)
    {
        result = HashCombine(result, binding.getHash());
    }

    return result;
}
