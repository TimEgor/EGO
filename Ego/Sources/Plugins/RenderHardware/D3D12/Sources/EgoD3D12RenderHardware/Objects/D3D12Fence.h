#pragma once

#include <wrl/client.h>

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Fence.h"

#include "EgoD3D12RenderHardware/Common/D3D12Utils.h"

namespace ego::gpu::d3d12
{
    class D3D12GraphicDevice;

    class D3D12Fence final : public Fence
    {
    public:
        D3D12Fence(
            FenceValue _initialValue,
            Microsoft::WRL::ComPtr<ID3D12Fence>&& _fence
        );
        ~D3D12Fence() override;

        virtual void* getNativeHandle() const override;
        virtual void setName(const char* _name) override;

        virtual FenceValue getCompletedValue() const override;
        virtual void waitValue(FenceValue _value) override;

        ID3D12Fence* getFence() const;

    private:
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        void* m_eventHandle = nullptr;
    };
}
