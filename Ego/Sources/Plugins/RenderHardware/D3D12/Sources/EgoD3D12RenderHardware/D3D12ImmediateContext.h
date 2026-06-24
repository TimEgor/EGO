#pragma once

#include <functional>
#include <vector>

#include <wrl/client.h>

#include "Common/D3D12Utils.h"

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/GpuTask.h"

namespace ego::gpu::d3d12
{
    class D3D12ImmediateContext final
    {
    public:
        D3D12ImmediateContext() = default;

        bool init(ID3D12Device5* _device);
        void release();
        bool wait();

        GpuTaskReference submit(
            const std::function<void(ID3D12GraphicsCommandList4*)>& _recordCommands,
            const std::vector<GraphicObjectReference>& _keepAliveObjects = std::vector<GraphicObjectReference>());
        bool execute(const std::function<void(ID3D12GraphicsCommandList4*)>& _recordCommands);

    private:
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_queue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_allocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> m_commandList;
        FenceReference m_fence = nullptr;
        uint64_t m_fenceValue = 0;
    };
} // namespace ego::gpu::d3d12
