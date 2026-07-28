#pragma once

#include <vector>

#include "EgoGraphicHardware/GraphicObjects/CommandQueue.h"
#include "EgoGraphicHardware/GraphicObjects/SwapChain.h"

#include <dxgi1_6.h>
#include <wrl/client.h>

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
            const CommandQueuePointer& _presentationQueue);

        void* getNativeHandle() const override;
        void setName(const char* _name) override;

        Texture2DPointer getTargetTexture() override;
        bool resize(const Texture2DSize& _size) override;
        void present() override;

    private:
        bool createTargetTextures(const Texture2DSize& _size);

        Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
        std::vector<Texture2DPointer> m_targetTextures;
        CommandQueuePointer m_presentationQueue;
    };
} // namespace ego::gpu::d3d12
