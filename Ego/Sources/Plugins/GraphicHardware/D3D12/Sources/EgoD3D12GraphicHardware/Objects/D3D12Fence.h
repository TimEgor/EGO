#pragma once

#include "EgoGraphicHardware/GraphicObjects/Fence.h"

#include "EgoD3D12GraphicHardware/Common/D3D12Utils.h"

#include <wrl/client.h>

namespace ego::gpu::d3d12
{
    class D3D12GraphicDevice;

    class D3D12Fence final : public Fence
    {
    public:
        D3D12Fence(FenceValue _initialValue, Microsoft::WRL::ComPtr<ID3D12Fence>&& _fence);
        ~D3D12Fence() override;

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        FenceValue getCompletedValue() const override;
        void waitValue(FenceValue _value) override;

        ID3D12Fence* getFence() const;

    private:
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        void* m_eventHandle = nullptr;
    };
} // namespace ego::gpu::d3d12
