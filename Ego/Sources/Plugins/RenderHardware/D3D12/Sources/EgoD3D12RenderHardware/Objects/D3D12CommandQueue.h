#pragma once

#include <wrl/client.h>

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/CommandQueue.h"

#include "EgoD3D12RenderHardware/Common/D3D12Utils.h"

namespace ego::gpu::d3d12
{
    class D3D12CommandQueue final : public CommandQueue
    {
    public:
        D3D12CommandQueue(const CommandQueueDesc& _desc, Microsoft::WRL::ComPtr<ID3D12CommandQueue>&& _queue);

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        void execute(const CommandListReference& _commandList) override;
        void execute(const std::vector<CommandListReference>& _commandLists) override;

        void signal(const FenceReference& _fence, Fence::FenceValue _value) override;
        void wait(const FenceReference& _fence, Fence::FenceValue _value) override;
        void waitIdle() override;

    private:
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_queue;
    };
} // namespace ego::gpu::d3d12
