#include "InputLayout.h"

#include "EgoCore/Hash/Crc32.h"
#include "EgoCore/Hash/HashCombine.h"

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

ego::gpu::InputLayoutBindingHash ego::gpu::InputLayoutBindingDesc::getHash() const
{
    InputLayoutBindingHash result = 0;

    result = HashCombine(result, m_slot);
    result = HashCombine(result, m_stride);
    result = HashCombine(result, static_cast<uint64_t>(m_type));
    result = HashCombine(result, m_instanceStepRate);

    return result;
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
