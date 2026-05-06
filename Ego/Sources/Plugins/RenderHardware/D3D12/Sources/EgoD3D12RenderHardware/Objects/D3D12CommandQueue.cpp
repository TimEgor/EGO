#include "D3D12CommandQueue.h"

#include <utility>

#include "D3D12Fence.h"

#include "../D3D12GraphicDevice.h"

#include "EgoCore/Assert/AssertCore.h"

ego::gpu::d3d12::D3D12CommandQueue::D3D12CommandQueue(
    D3D12GraphicDevice* _device,
    const CommandQueueDesc& _desc,
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>&& _queue
)
    : CommandQueue(_desc),
      m_device(_device),
      m_queue(std::move(_queue))
{
    if (m_device)
    {
        m_device->registerQueue(this);
    }
}

ego::gpu::d3d12::D3D12CommandQueue::~D3D12CommandQueue()
{
    if (m_device)
    {
        m_device->unregisterQueue(this);
    }
}

void* ego::gpu::d3d12::D3D12CommandQueue::getNativeHandle() const
{
    return m_queue.Get();
}

void ego::gpu::d3d12::D3D12CommandQueue::setName(const char* _name)
{
    SetD3D12ObjectName(m_queue.Get(), _name);
}

void ego::gpu::d3d12::D3D12CommandQueue::execute(const CommandListPointer& _commandList)
{
    execute(std::vector<CommandListPointer>{_commandList});
}

void ego::gpu::d3d12::D3D12CommandQueue::execute(const std::vector<CommandListPointer>& _commandLists)
{
    std::vector<ID3D12CommandList*> nativeCommandLists;
    nativeCommandLists.reserve(_commandLists.size());

    for (const CommandListPointer& commandList : _commandLists)
    {
        ID3D12GraphicsCommandList* nativeCommandList =
            commandList ? commandList->getNativeHandle<ID3D12GraphicsCommandList>() : nullptr;
        EGO_ASSERT_MESSAGE(nativeCommandList, "CommandList must be created by D3D12 device");
        if (nativeCommandList)
        {
            nativeCommandLists.push_back(nativeCommandList);
        }
    }

    if (!nativeCommandLists.empty())
    {
        m_queue->ExecuteCommandLists(static_cast<UINT>(nativeCommandLists.size()), nativeCommandLists.data());
    }
}

void ego::gpu::d3d12::D3D12CommandQueue::signal(const FencePointer& _fence, Fence::FenceValue _value)
{
    ID3D12Fence* nativeFence = _fence ? _fence->getNativeHandle<ID3D12Fence>() : nullptr;
    EGO_ASSERT_MESSAGE(nativeFence, "Fence must be created by D3D12 device");
    if (nativeFence)
    {
        m_queue->Signal(nativeFence, _value);
    }
}

void ego::gpu::d3d12::D3D12CommandQueue::wait(const FencePointer& _fence, Fence::FenceValue _value)
{
    ID3D12Fence* nativeFence = _fence ? _fence->getNativeHandle<ID3D12Fence>() : nullptr;
    EGO_ASSERT_MESSAGE(nativeFence, "Fence must be created by D3D12 device");
    if (nativeFence)
    {
        m_queue->Wait(nativeFence, _value);
    }
}

void ego::gpu::d3d12::D3D12CommandQueue::waitIdle()
{
    Microsoft::WRL::ComPtr<ID3D12Fence> nativeFence;
    if (FAILED(m_device->getDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&nativeFence))))
    {
        return;
    }

    D3D12Fence tempFence(0, std::move(nativeFence));
    const Fence::FenceValue waitValue = tempFence.getCompletedValue() + 1;
    m_queue->Signal(tempFence.getFence(), waitValue);
    tempFence.waitValue(waitValue);
}

ID3D12CommandQueue* ego::gpu::d3d12::D3D12CommandQueue::getQueue() const
{
    return m_queue.Get();
}
