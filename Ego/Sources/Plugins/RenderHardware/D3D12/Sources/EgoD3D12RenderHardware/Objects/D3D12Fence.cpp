#include "D3D12Fence.h"

#include <utility>

ego::gpu::d3d12::D3D12Fence::D3D12Fence(FenceValue, Microsoft::WRL::ComPtr<ID3D12Fence>&& _fence)
    : m_fence(std::move(_fence))
{
    m_eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

ego::gpu::d3d12::D3D12Fence::~D3D12Fence()
{
    if (m_eventHandle)
    {
        CloseHandle(static_cast<HANDLE>(m_eventHandle));
    }
}

void* ego::gpu::d3d12::D3D12Fence::getNativeHandle() const
{
    return m_fence.Get();
}

void ego::gpu::d3d12::D3D12Fence::setName(const char* _name)
{
    SetD3D12ObjectName(m_fence.Get(), _name);
}

ego::gpu::Fence::FenceValue ego::gpu::d3d12::D3D12Fence::getCompletedValue() const
{
    return m_fence ? m_fence->GetCompletedValue() : 0;
}

void ego::gpu::d3d12::D3D12Fence::waitValue(FenceValue _value)
{
    if (!m_fence || getCompletedValue() >= _value)
    {
        return;
    }

    m_fence->SetEventOnCompletion(_value, static_cast<HANDLE>(m_eventHandle));
    WaitForSingleObject(static_cast<HANDLE>(m_eventHandle), INFINITE);
}

ID3D12Fence* ego::gpu::d3d12::D3D12Fence::getFence() const
{
    return m_fence.Get();
}
