#include "D3D12Shader.h"

template <typename TBaseShader>
ego::gpu::d3d12::D3D12Shader<TBaseShader>::D3D12Shader(const ShaderCodePointer& _code)
    : TBaseShader(_code)
{
}

template <typename TBaseShader>
void* ego::gpu::d3d12::D3D12Shader<TBaseShader>::getNativeHandle() const
{
    return this->m_code ? this->m_code->getCode() : nullptr;
}

template <typename TBaseShader>
void ego::gpu::d3d12::D3D12Shader<TBaseShader>::setName(const char* _name)
{
}

template <typename TBaseShader>
D3D12_SHADER_BYTECODE ego::gpu::d3d12::D3D12Shader<TBaseShader>::getD3D12ByteCode() const
{
    D3D12_SHADER_BYTECODE byteCode = {};
    byteCode.pShaderBytecode = this->m_code ? this->m_code->getCode() : nullptr;
    byteCode.BytecodeLength = this->m_code ? this->m_code->getCodeSize() : 0;
    return byteCode;
}

template class ego::gpu::d3d12::D3D12Shader<ego::gpu::VertexShader>;
template class ego::gpu::d3d12::D3D12Shader<ego::gpu::PixelShader>;
template class ego::gpu::d3d12::D3D12Shader<ego::gpu::ComputeShader>;
template class ego::gpu::d3d12::D3D12Shader<ego::gpu::RayGenerationShader>;
template class ego::gpu::d3d12::D3D12Shader<ego::gpu::MissShader>;
template class ego::gpu::d3d12::D3D12Shader<ego::gpu::ClosestHitShader>;
template class ego::gpu::d3d12::D3D12Shader<ego::gpu::AnyHitShader>;
template class ego::gpu::d3d12::D3D12Shader<ego::gpu::IntersectionShader>;
