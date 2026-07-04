#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "Shader.h"

namespace ego::gpu
{
    inline constexpr uint32_t MaterialParameterBufferShaderRegister = 1;
    inline constexpr uint32_t MaterialParameterBufferRegisterSpace = 0;

    enum class ShaderParameterScalarType
    {
        Unknown,
        Bool,
        Int,
        UInt,
        Float
    };

    struct ShaderParameterType final
    {
        ShaderParameterScalarType m_scalarType = ShaderParameterScalarType::Unknown;
        uint32_t m_rowCount = 0;
        uint32_t m_columnCount = 0;
        uint32_t m_elementCount = 0;
        bool m_columnMajor = true;

        bool operator==(const ShaderParameterType& _other) const;
        bool operator!=(const ShaderParameterType& _other) const;
    };

    struct ShaderParameterDesc final
    {
        std::string m_name;
        uint32_t m_offset = 0;
        uint32_t m_size = 0;
        ShaderStageFlags m_stageFlags = ShaderStageFlagNone;
        ShaderParameterType m_type;
    };

    struct ShaderParameterBufferDesc final
    {
        std::string m_name;
        uint32_t m_size = 0;
        uint32_t m_shaderRegister = MaterialParameterBufferShaderRegister;
        uint32_t m_registerSpace = MaterialParameterBufferRegisterSpace;
        ShaderStageFlags m_stageFlags = ShaderStageFlagNone;
        std::vector<ShaderParameterDesc> m_parameters;
    };

    class ShaderInterface final
    {
    public:
        bool hasMaterialParameterBuffer() const;

        const ShaderParameterBufferDesc& getMaterialParameterBuffer() const;
        ShaderParameterBufferDesc& getMaterialParameterBuffer();

        const ShaderParameterDesc* findMaterialParameter(std::string_view _name) const;
        ShaderParameterDesc* findMaterialParameter(std::string_view _name);

        bool merge(const ShaderInterface& _other);

    private:
        ShaderParameterBufferDesc m_materialParameterBuffer;
    };
} // namespace ego::gpu
