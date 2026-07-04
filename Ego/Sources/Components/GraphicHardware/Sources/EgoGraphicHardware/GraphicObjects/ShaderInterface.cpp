#include "ShaderInterface.h"

bool ego::gpu::ShaderParameterType::operator==(const ShaderParameterType& _other) const
{
    return m_scalarType == _other.m_scalarType && m_rowCount == _other.m_rowCount && m_columnCount == _other.m_columnCount && m_elementCount == _other.m_elementCount &&
           m_columnMajor == _other.m_columnMajor;
}

bool ego::gpu::ShaderParameterType::operator!=(const ShaderParameterType& _other) const
{
    return !(*this == _other);
}

bool ego::gpu::ShaderInterface::hasMaterialParameterBuffer() const
{
    return m_materialParameterBuffer.m_size != 0;
}

const ego::gpu::ShaderParameterBufferDesc& ego::gpu::ShaderInterface::getMaterialParameterBuffer() const
{
    return m_materialParameterBuffer;
}

ego::gpu::ShaderParameterBufferDesc& ego::gpu::ShaderInterface::getMaterialParameterBuffer()
{
    return m_materialParameterBuffer;
}

const ego::gpu::ShaderParameterDesc* ego::gpu::ShaderInterface::findMaterialParameter(std::string_view _name) const
{
    for (const ShaderParameterDesc& parameter : m_materialParameterBuffer.m_parameters)
    {
        if (parameter.m_name == _name)
        {
            return &parameter;
        }
    }

    return nullptr;
}

ego::gpu::ShaderParameterDesc* ego::gpu::ShaderInterface::findMaterialParameter(std::string_view _name)
{
    for (ShaderParameterDesc& parameter : m_materialParameterBuffer.m_parameters)
    {
        if (parameter.m_name == _name)
        {
            return &parameter;
        }
    }

    return nullptr;
}

bool ego::gpu::ShaderInterface::merge(const ShaderInterface& _other)
{
    if (!_other.hasMaterialParameterBuffer())
    {
        return true;
    }

    if (!hasMaterialParameterBuffer())
    {
        m_materialParameterBuffer = _other.m_materialParameterBuffer;
        return true;
    }

    const ShaderParameterBufferDesc& otherBuffer = _other.m_materialParameterBuffer;
    if (m_materialParameterBuffer.m_size != otherBuffer.m_size || m_materialParameterBuffer.m_shaderRegister != otherBuffer.m_shaderRegister ||
        m_materialParameterBuffer.m_registerSpace != otherBuffer.m_registerSpace)
    {
        return false;
    }

    m_materialParameterBuffer.m_stageFlags |= otherBuffer.m_stageFlags;

    for (const ShaderParameterDesc& otherParameter : otherBuffer.m_parameters)
    {
        ShaderParameterDesc* parameter = findMaterialParameter(otherParameter.m_name);
        if (!parameter)
        {
            m_materialParameterBuffer.m_parameters.push_back(otherParameter);
            continue;
        }

        if (parameter->m_offset != otherParameter.m_offset || parameter->m_size != otherParameter.m_size || parameter->m_type != otherParameter.m_type)
        {
            return false;
        }

        parameter->m_stageFlags |= otherParameter.m_stageFlags;
    }

    return true;
}
