#pragma once

#include <vector>

#include <dxgi1_6.h>
#include <wrl/client.h>

#include "EgoEngine/RenderHardware/GraphicObjects/CommandQueue.h"
#include "EgoEngine/RenderHardware/GraphicObjects/SwapChain.h"

namespace ego::gpu::d3d12
{
    class D3D12GraphicDevice;

    class D3D12SwapChain final : public SwapChain
    {
    public:
        D3D12SwapChain(
            const SwapChainDesc& _desc,
            Microsoft::WRL::ComPtr<IDXGISwapChain3>&& _swapChain,
            std::vector<Texture2DPointer>&& _targetTextures,
            CommandQueuePointer&& _ownedPresentationQueue
        );

        virtual void* getNativeHandle() const override;
        virtual void setName(const char* _name) override;

        virtual Texture2DPointer getTargetTexture() override;
        virtual void present() override;

    private:
        Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
        std::vector<Texture2DPointer> m_targetTextures;
        CommandQueuePointer m_ownedPresentationQueue;
    };
}
