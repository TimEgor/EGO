#include "ShaderPackage.h"

#include <cstring>
#include <limits>
#include <utility>

namespace
{
    constexpr uint32_t ShaderPackageMagic = 0x50534745;
    constexpr uint32_t ShaderPackageVersion = 1;

    void AppendUInt32(ego::FileContent& _content, uint32_t _value)
    {
        _content.push_back(static_cast<uint8_t>(_value & 0xFF));
        _content.push_back(static_cast<uint8_t>((_value >> 8) & 0xFF));
        _content.push_back(static_cast<uint8_t>((_value >> 16) & 0xFF));
        _content.push_back(static_cast<uint8_t>((_value >> 24) & 0xFF));
    }

    void AppendBytes(ego::FileContent& _content, const void* _data, size_t _size)
    {
        if (!_data || _size == 0)
        {
            return;
        }

        const uint8_t* bytes = static_cast<const uint8_t*>(_data);
        _content.insert(_content.end(), bytes, bytes + _size);
    }

    void AppendString(ego::FileContent& _content, const std::string& _value)
    {
        AppendUInt32(_content, static_cast<uint32_t>(_value.size()));
        AppendBytes(_content, _value.data(), _value.size());
    }

    bool ReadUInt32(const ego::FileContent& _content, size_t& _offset, uint32_t& _value)
    {
        if (_offset + sizeof(uint32_t) > _content.size())
        {
            return false;
        }

        _value =
            static_cast<uint32_t>(_content[_offset]) |
            (static_cast<uint32_t>(_content[_offset + 1]) << 8) |
            (static_cast<uint32_t>(_content[_offset + 2]) << 16) |
            (static_cast<uint32_t>(_content[_offset + 3]) << 24);
        _offset += sizeof(uint32_t);
        return true;
    }

    bool ReadString(const ego::FileContent& _content, size_t& _offset, std::string& _value)
    {
        uint32_t size = 0;
        if (!ReadUInt32(_content, _offset, size) || _offset + size > _content.size())
        {
            return false;
        }

        _value.assign(
            reinterpret_cast<const char*>(_content.data() + _offset),
            reinterpret_cast<const char*>(_content.data() + _offset + size)
        );
        _offset += size;
        return true;
    }
}

ego::FileContent ego::gpu::PackShaderContent(
    const FileContent& _shaderCode,
    const ShaderInterface& _shaderInterface
)
{
    FileContent content;
    const ShaderParameterBufferDesc& materialBuffer = _shaderInterface.getMaterialParameterBuffer();
    const uint32_t parameterCount = static_cast<uint32_t>(materialBuffer.m_parameters.size());

    content.reserve(
        sizeof(uint32_t) * 8 +
        materialBuffer.m_name.size() +
        parameterCount * sizeof(uint32_t) * 9 +
        _shaderCode.size()
    );

    AppendUInt32(content, ShaderPackageMagic);
    AppendUInt32(content, ShaderPackageVersion);
    AppendUInt32(content, static_cast<uint32_t>(_shaderCode.size()));
    AppendUInt32(content, materialBuffer.m_size);
    AppendUInt32(content, materialBuffer.m_shaderRegister);
    AppendUInt32(content, materialBuffer.m_registerSpace);
    AppendUInt32(content, materialBuffer.m_stageFlags);
    AppendUInt32(content, parameterCount);
    AppendString(content, materialBuffer.m_name);

    for (const ShaderParameterDesc& parameter : materialBuffer.m_parameters)
    {
        AppendString(content, parameter.m_name);
        AppendUInt32(content, parameter.m_offset);
        AppendUInt32(content, parameter.m_size);
        AppendUInt32(content, parameter.m_stageFlags);
        AppendUInt32(content, static_cast<uint32_t>(parameter.m_type.m_scalarType));
        AppendUInt32(content, parameter.m_type.m_rowCount);
        AppendUInt32(content, parameter.m_type.m_columnCount);
        AppendUInt32(content, parameter.m_type.m_elementCount);
        AppendUInt32(content, parameter.m_type.m_columnMajor ? 1u : 0u);
    }

    AppendBytes(content, _shaderCode.data(), _shaderCode.size());
    return content;
}

bool ego::gpu::IsShaderPackageContent(const FileContent& _content)
{
    if (_content.size() < sizeof(uint32_t))
    {
        return false;
    }

    size_t offset = 0;
    uint32_t magic = 0;
    return ReadUInt32(_content, offset, magic) && magic == ShaderPackageMagic;
}

bool ego::gpu::TryUnpackShaderContent(
    const FileContent& _content,
    FileContent& _shaderCode,
    ShaderInterface& _shaderInterface
)
{
    _shaderCode.clear();
    _shaderInterface = ShaderInterface();

    size_t offset = 0;
    uint32_t magic = 0;
    uint32_t version = 0;
    uint32_t shaderCodeSize = 0;
    uint32_t materialBufferSize = 0;
    uint32_t materialBufferShaderRegister = 0;
    uint32_t materialBufferRegisterSpace = 0;
    uint32_t materialBufferStageFlags = 0;
    uint32_t parameterCount = 0;

    if (!ReadUInt32(_content, offset, magic) ||
        !ReadUInt32(_content, offset, version) ||
        !ReadUInt32(_content, offset, shaderCodeSize) ||
        !ReadUInt32(_content, offset, materialBufferSize) ||
        !ReadUInt32(_content, offset, materialBufferShaderRegister) ||
        !ReadUInt32(_content, offset, materialBufferRegisterSpace) ||
        !ReadUInt32(_content, offset, materialBufferStageFlags) ||
        !ReadUInt32(_content, offset, parameterCount) ||
        magic != ShaderPackageMagic ||
        version != ShaderPackageVersion)
    {
        return false;
    }

    ShaderParameterBufferDesc& materialBuffer = _shaderInterface.getMaterialParameterBuffer();
    materialBuffer.m_size = materialBufferSize;
    materialBuffer.m_shaderRegister = materialBufferShaderRegister;
    materialBuffer.m_registerSpace = materialBufferRegisterSpace;
    materialBuffer.m_stageFlags = materialBufferStageFlags;

    if (!ReadString(_content, offset, materialBuffer.m_name))
    {
        return false;
    }

    materialBuffer.m_parameters.reserve(parameterCount);
    for (uint32_t parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex)
    {
        ShaderParameterDesc parameter;
        uint32_t scalarType = 0;
        uint32_t columnMajor = 0;

        if (!ReadString(_content, offset, parameter.m_name) ||
            !ReadUInt32(_content, offset, parameter.m_offset) ||
            !ReadUInt32(_content, offset, parameter.m_size) ||
            !ReadUInt32(_content, offset, parameter.m_stageFlags) ||
            !ReadUInt32(_content, offset, scalarType) ||
            !ReadUInt32(_content, offset, parameter.m_type.m_rowCount) ||
            !ReadUInt32(_content, offset, parameter.m_type.m_columnCount) ||
            !ReadUInt32(_content, offset, parameter.m_type.m_elementCount) ||
            !ReadUInt32(_content, offset, columnMajor))
        {
            return false;
        }

        parameter.m_type.m_scalarType = static_cast<ShaderParameterScalarType>(scalarType);
        parameter.m_type.m_columnMajor = columnMajor != 0;
        materialBuffer.m_parameters.push_back(std::move(parameter));
    }

    if (offset + shaderCodeSize != _content.size())
    {
        return false;
    }

    _shaderCode.resize(shaderCodeSize);
    if (shaderCodeSize != 0)
    {
        std::memcpy(_shaderCode.data(), _content.data() + offset, shaderCodeSize);
    }

    return true;
}
