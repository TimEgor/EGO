#pragma once

#include <cstdint>
#include <vector>

namespace ego::gpu
{
    using InputLayoutElementNameHash = uint32_t;
    using InputLayoutElementHash = uint32_t;

    enum class InputLayoutElementType
    {
        Int8,
        Int16,
        Int32,
        UInt8,
        UInt16,
        UInt32,
        Float16,
        Float32,

        Undefined
    };

    using InputLayoutElementSemanticNameType = const char*;

    struct InputLayoutElementDesc final
    {
        InputLayoutElementSemanticNameType m_semanticName = nullptr;
        uint32_t m_location = 0;
        uint32_t m_index = 0;
        uint32_t m_slot = 0;
        uint32_t m_offset = 0;
        uint32_t m_componentsCount = 0;
        InputLayoutElementType m_type = InputLayoutElementType::Undefined;

        bool operator==(const InputLayoutElementDesc& _other) const;
        bool operator!=(const InputLayoutElementDesc& _other) const;

        InputLayoutElementHash getHash() const;
    };

    enum class InputLayoutBindingType
    {
        VertexBinding,
        InstanceBinding
    };

    using InputLayoutBindingHash = uint32_t;

    struct InputLayoutBindingDesc final
    {
        uint32_t m_slot = 0;
        uint32_t m_stride = 0;
        InputLayoutBindingType m_type = InputLayoutBindingType::VertexBinding;
        uint32_t m_instanceStepRate = 1;

        bool operator==(const InputLayoutBindingDesc& _other) const;
        bool operator!=(const InputLayoutBindingDesc& _other) const;

        InputLayoutBindingHash getHash() const;
    };

    using InputLayoutHash = uint32_t;

    struct InputLayoutDesc final
    {
        using ElementDescContainer = std::vector<InputLayoutElementDesc>;
        using BindingDescContainer = std::vector<InputLayoutBindingDesc>;

        ElementDescContainer m_elements;
        BindingDescContainer m_bindings;

        bool operator==(const InputLayoutDesc& _other) const;
        bool operator!=(const InputLayoutDesc& _other) const;

        InputLayoutHash getHash() const;
    };
} // namespace ego::gpu
