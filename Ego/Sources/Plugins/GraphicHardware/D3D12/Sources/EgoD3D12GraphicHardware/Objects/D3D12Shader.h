#pragma once

#include "EgoGraphicHardware/GraphicObjects/Shader.h"

#include "EgoD3D12GraphicHardware/Common/D3D12Utils.h"

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
    class D3D12Shader final : public TBaseShader, public D3D12ShaderAccess
    {
    public:
        D3D12Shader(const ShaderCodeReference& _code);

        virtual void* getNativeHandle() const override;
        virtual void setName(const char* _name) override;

        D3D12_SHADER_BYTECODE getD3D12ByteCode() const override;
    };

    using D3D12VertexShader = D3D12Shader<VertexShader>;
    using D3D12PixelShader = D3D12Shader<PixelShader>;
    using D3D12ComputeShader = D3D12Shader<ComputeShader>;
    using D3D12RayGenerationShader = D3D12Shader<RayGenerationShader>;
    using D3D12MissShader = D3D12Shader<MissShader>;
    using D3D12ClosestHitShader = D3D12Shader<ClosestHitShader>;
    using D3D12AnyHitShader = D3D12Shader<AnyHitShader>;
    using D3D12IntersectionShader = D3D12Shader<IntersectionShader>;
} // namespace ego::gpu::d3d12
