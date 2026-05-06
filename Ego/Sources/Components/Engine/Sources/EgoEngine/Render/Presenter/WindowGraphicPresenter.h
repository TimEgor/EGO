#pragma once

#include "EgoEngine/Platform/Window/Window.h"
#include "EgoEngine/RenderHardware/GraphicObjects/SwapChain.h"

#include "GraphicPresenter.h"

namespace ego
{
    class WindowGraphicPresenter final : public GraphicPresenter
    {
    public:
        WindowGraphicPresenter() = default;
        ~WindowGraphicPresenter() override;

        bool init(const Window& _window, const gpu::SwapChainDesc& _swapChainDesc);
        void release();

        virtual gpu::Texture2DPointer getTargetTexture() override;

        virtual void present() override;

    private:
        gpu::SwapChainPointer m_swapChain;
    };
}
