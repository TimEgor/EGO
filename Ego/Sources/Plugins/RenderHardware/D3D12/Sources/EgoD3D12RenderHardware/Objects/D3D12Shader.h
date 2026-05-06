#pragma once

#include "EgoEngine/RenderHardware/GraphicObjects/Shader.h"

#include "EgoD3D12RenderHardware/Common/D3D12Utils.h"

namespace ego::gpu::d3d12
{
    class D3D12ShaderAccess
    {
    public:
        virtual D3D12_SHADER_BYTECODE getD3D12ByteCode() const = 0;

    protected:
        ~D3D12ShaderAccess() = default;
    };

    template <typename TBaseShader>
    class D3D12Shader final : public TBaseShader,
                              public D3D12ShaderAccess
    {
    public:
        D3D12Shader(const ShaderCodePointer& _code);

        virtual void* getNativeHandle() const override;
        virtual void setName(const char* _name) override;

        virtual D3D12_SHADER_BYTECODE getD3D12ByteCode() const override;
    };

    using D3D12VertexShader = D3D12Shader<VertexShader>;
    using D3D12PixelShader = D3D12Shader<PixelShader>;
    using D3D12ComputeShader = D3D12Shader<ComputeShader>;
}
