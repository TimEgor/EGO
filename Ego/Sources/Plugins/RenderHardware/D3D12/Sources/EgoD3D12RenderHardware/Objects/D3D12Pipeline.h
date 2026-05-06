#pragma once

#include <string>
#include <vector>

#include <wrl/client.h>

#include "EgoEngine/RenderHardware/GraphicObjects/BindingLayout.h"
#include "EgoEngine/RenderHardware/GraphicObjects/Pipeline.h"
#include "EgoEngine/RenderHardware/GraphicObjects/Shader.h"

#include "../Common/D3D12DescriptorAllocator.h"

namespace ego::gpu::d3d12
{
    class D3D12GraphicDevice;

    class D3D12BindingLayout final : public BindingLayout
    {
    public:
        struct DescriptorRangeInfo final
        {
            uint32_t m_binding = 0;
            uint32_t m_descriptorCount = 0;
            uint32_t m_baseOffset = 0;
            BindingResourceType m_type = BindingResourceType::Texture;
        };

        struct SetInfo final
        {
            uint32_t m_set = 0;
            uint32_t m_resourceRootParameterIndex = InvalidRootParameterIndex;
            uint32_t m_samplerRootParameterIndex = InvalidRootParameterIndex;
            std::vector<DescriptorRangeInfo> m_resourceRanges;
            std::vector<DescriptorRangeInfo> m_samplerRanges;

            uint32_t getResourceDescriptorCount() const;
            uint32_t getSamplerDescriptorCount() const;
        };

        struct PushConstantInfo final
        {
            uint32_t m_rootParameterIndex = InvalidRootParameterIndex;
            uint32_t m_offset = 0;
            uint32_t m_size = 0;
            ShaderStageFlags m_stageFlags = ShaderStageFlagNone;
        };

        D3D12BindingLayout(
            const BindingLayoutDesc& _desc,
            Microsoft::WRL::ComPtr<ID3D12RootSignature>&& _rootSignature,
            std::vector<SetInfo>&& _setInfos,
            std::vector<PushConstantInfo>&& _pushConstants
        );

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        ID3D12RootSignature* getRootSignature() const;

        const SetInfo* findSetInfo(uint32_t _set) const;

        const PushConstantInfo* findPushConstantRange(
            ShaderStageFlags _stageFlags,
            uint32_t _offset,
            uint32_t _size
        ) const;

    private:
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
        std::vector<SetInfo> m_setInfos;
        std::vector<PushConstantInfo> m_pushConstants;
    };

    class D3D12BindingSet final : public BindingSet
    {
    public:
        D3D12BindingSet(
            const BindingSetDesc& _desc,
            const D3D12BindingLayout* _layout,
            const D3D12BindingLayout::SetInfo* _setInfo
        );
        ~D3D12BindingSet() override;

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        const D3D12BindingLayout* getD3D12Layout() const;
        const D3D12BindingLayout::SetInfo* getSetInfo() const;

        bool hasResourceDescriptors() const;
        bool hasSamplerDescriptors() const;

        D3D12_GPU_DESCRIPTOR_HANDLE getResourceGpuHandle() const;
        D3D12_GPU_DESCRIPTOR_HANDLE getSamplerGpuHandle() const;

    private:
        const D3D12BindingLayout* m_layout = nullptr;
        const D3D12BindingLayout::SetInfo* m_setInfo = nullptr;
    };

    class D3D12GraphicPipeline final : public GraphicPipeline
    {
    public:
        D3D12GraphicPipeline(
            const GraphicPipelineDesc& _desc,
            Microsoft::WRL::ComPtr<ID3D12PipelineState>&& _pipelineState,
            const D3D12BindingLayout* _layout
        );

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
        D3D12ComputePipeline(
            const ComputePipelineDesc& _desc,
            Microsoft::WRL::ComPtr<ID3D12PipelineState>&& _pipelineState,
            const D3D12BindingLayout* _layout
        );

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        ID3D12PipelineState* getPipelineState() const;
        const D3D12BindingLayout* getBindingLayout() const;

    private:
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
        const D3D12BindingLayout* m_layout = nullptr;
    };
}
