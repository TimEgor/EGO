#include "D3D12ImmediateContext.h"

#include "Objects/D3D12Fence.h"

#include "EgoCore/Assert/AssertCore.h"

bool ego::gpu::d3d12::D3D12ImmediateContext::init(ID3D12Device5* _device)
{
    if (!_device)
    {
        return false;
    }

    release();

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    if (FAILED(_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queue))) ||
        FAILED(_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_allocator))) ||
        FAILED(_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList))) ||
        FAILED(_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
    {
        release();
        return false;
    }

    if (FAILED(m_commandList->Close()))
    {
        release();
        return false;
    }

    m_fence = FenceReference(new D3D12Fence(0, std::move(fence)));
    m_fenceValue = 0;
    return true;
}

void ego::gpu::d3d12::D3D12ImmediateContext::release()
{
    wait();

    m_commandList.Reset();
    m_allocator.Reset();
    m_queue.Reset();
    m_fence.reset();
    m_fenceValue = 0;
}

bool ego::gpu::d3d12::D3D12ImmediateContext::wait()
{
    if (!m_fence || m_fenceValue == 0)
    {
        return true;
    }

    if (m_fence->getCompletedValue() >= m_fenceValue)
    {
        return true;
    }

    m_fence->waitValue(m_fenceValue);
    return true;
}

ego::gpu::GpuTaskReference ego::gpu::d3d12::D3D12ImmediateContext::submit(
    const std::function<void(ID3D12GraphicsCommandList4*)>& _recordCommands,
    const std::vector<GraphicObjectReference>& _keepAliveObjects)
{
    if (!_recordCommands || !m_queue || !m_allocator || !m_commandList || !m_fence)
    {
        return GpuTaskReference();
    }

    EGO_CHECK_RETURN_VALUE(wait(), GpuTaskReference());
    EGO_CHECK_RETURN_VALUE(SUCCEEDED(m_allocator->Reset()), GpuTaskReference());
    EGO_CHECK_RETURN_VALUE(SUCCEEDED(m_commandList->Reset(m_allocator.Get(), nullptr)), GpuTaskReference());

    _recordCommands(m_commandList.Get());
    EGO_CHECK_RETURN_VALUE(SUCCEEDED(m_commandList->Close()), GpuTaskReference());

    ID3D12CommandList* commandLists[] = {m_commandList.Get()};
    m_queue->ExecuteCommandLists(1, commandLists);

    ++m_fenceValue;
    ID3D12Fence* fence = m_fence->getNativeHandle<ID3D12Fence>();
    EGO_CHECK_RETURN_VALUE(fence, GpuTaskReference());
    EGO_CHECK_RETURN_VALUE(SUCCEEDED(m_queue->Signal(fence, m_fenceValue)), GpuTaskReference());

    GpuTaskReference task(new GpuTask());
    GpuSyncPoint syncPoint;
    syncPoint.m_queueType = CommandType::Graphic;
    syncPoint.m_fence = m_fence;
    syncPoint.m_value = m_fenceValue;
    task->addSyncPoint(syncPoint);

    for (const GraphicObjectReference& keepAliveObject : _keepAliveObjects)
    {
        task->addKeepAliveObject(keepAliveObject);
    }

    return task;
}

bool ego::gpu::d3d12::D3D12ImmediateContext::execute(const std::function<void(ID3D12GraphicsCommandList4*)>& _recordCommands)
{
    GpuTaskReference task = submit(_recordCommands);
    if (!task)
    {
        return false;
    }

    task->wait();
    return true;
}
