#pragma once

#include <wrl/client.h>

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/CommandQueue.h"

#include "EgoD3D12RenderHardware/Common/D3D12Utils.h"

namespace ego::gpu::d3d12
{
    class D3D12GraphicDevice;

    class D3D12CommandQueue final : public CommandQueue
    {
    public:
        D3D12CommandQueue(
            D3D12GraphicDevice* _device,
            const CommandQueueDesc& _desc,
            Microsoft::WRL::ComPtr<ID3D12CommandQueue>&& _queue
        );
        ~D3D12CommandQueue() override;

        virtual void* getNativeHandle() const override;
        virtual void setName(const char* _name) override;

        virtual void execute(const CommandListReference& _commandList) override;
        virtual void execute(const std::vector<CommandListReference>& _commandLists) override;

        virtual void signal(const FenceReference& _fence, Fence::FenceValue _value) override;
        virtual void wait(const FenceReference& _fence, Fence::FenceValue _value) override;
        virtual void waitIdle() override;

        ID3D12CommandQueue* getQueue() const;

    private:
        D3D12GraphicDevice* m_device = nullptr;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_queue;
    };
}
