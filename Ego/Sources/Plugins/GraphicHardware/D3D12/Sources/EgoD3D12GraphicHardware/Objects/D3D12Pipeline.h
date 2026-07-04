#pragma once

#include <string>
#include <vector>

#include "EgoGraphicHardware/GraphicObjects/BindingLayout.h"
#include "EgoGraphicHardware/GraphicObjects/Pipeline.h"
#include "EgoGraphicHardware/GraphicObjects/Shader.h"

#include "../Common/D3D12Utils.h"

#include <wrl/client.h>

namespace ego::gpu::d3d12
{
    class D3D12GraphicDevice;

    class D3D12BindingLayout final : public BindingLayout
    {
    public:
        struct PushConstantInfo final
        {
            uint32_t m_rootParameterIndex = InvalidRootParameterIndex;
            uint32_t m_offset = 0;
            uint32_t m_size = 0;
            ShaderStageFlags m_stageFlags = ShaderStageFlagNone;
        };

        D3D12BindingLayout(const BindingLayoutDesc& _desc, Microsoft::WRL::ComPtr<ID3D12RootSignature>&& _rootSignature, std::vector<PushConstantInfo>&& _pushConstants);

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        ID3D12RootSignature* getRootSignature() const;

        const PushConstantInfo* findPushConstantRange(ShaderStageFlags _stageFlags, uint32_t _offset, uint32_t _size) const;

    private:
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
        std::vector<PushConstantInfo> m_pushConstants;
    };

    class D3D12GraphicPipeline final : public GraphicPipeline
    {
    public:
        D3D12GraphicPipeline(const GraphicPipelineDesc& _desc, Microsoft::WRL::ComPtr<ID3D12PipelineState>&& _pipelineState, const D3D12BindingLayout* _layout);

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        ID3D12PipelineState* getPipelineState() const;
        const D3D12BindingLayout* getBindingLayout() const;
        D3D_PRIMITIVE_TOPOLOGY getPrimitiveTopology() const;

    private:
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
        const D3D12BindingLayout* m_layout = nullptr;
        D3D_PRIMITIVE_TOPOLOGY m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    };

    class D3D12ComputePipeline final : public ComputePipeline
    {
    public:
        D3D12ComputePipeline(const ComputePipelineDesc& _desc, Microsoft::WRL::ComPtr<ID3D12PipelineState>&& _pipelineState, const D3D12BindingLayout* _layout);

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        ID3D12PipelineState* getPipelineState() const;
        const D3D12BindingLayout* getBindingLayout() const;

    private:
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
        const D3D12BindingLayout* m_layout = nullptr;
    };

    class D3D12RayTracingPipeline final : public RayTracingPipeline
    {
    public:
        D3D12RayTracingPipeline(
            const RayTracingPipelineDesc& _desc,
            Microsoft::WRL::ComPtr<ID3D12StateObject>&& _stateObject,
            Microsoft::WRL::ComPtr<ID3D12Resource>&& _shaderTable,
            uint64_t _shaderRecordSize,
            uint32_t _hitGroupCount,
            const D3D12BindingLayout* _layout);

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        ID3D12StateObject* getStateObject() const;
        const D3D12BindingLayout* getBindingLayout() const;
        D3D12_DISPATCH_RAYS_DESC getDispatchRaysDesc(uint32_t _width, uint32_t _height, uint32_t _depth) const;

    private:
        Microsoft::WRL::ComPtr<ID3D12StateObject> m_stateObject;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_shaderTable;
        uint64_t m_shaderRecordSize = 0;
        uint32_t m_hitGroupCount = 0;
        const D3D12BindingLayout* m_layout = nullptr;
    };
} // namespace ego::gpu::d3d12
