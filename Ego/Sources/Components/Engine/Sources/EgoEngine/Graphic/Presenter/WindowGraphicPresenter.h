#pragma once

#include "EgoGraphicHardware/GraphicDevice.h"
#include "EgoGraphicHardware/GraphicObjects/CommandQueue.h"
#include "EgoGraphicHardware/GraphicObjects/SwapChain.h"
#include "EgoGraphicHardware/PresentationSurface.h"
#include "GraphicPresenter.h"

namespace ego
{
    class WindowGraphicPresenter final : public GraphicPresenter
    {
    public:
        WindowGraphicPresenter() = default;
        ~WindowGraphicPresenter() override;

        bool init(
            GraphicDevice& _graphicDevice,
            const PresentationSurface& _surface,
            const gpu::SwapChainDesc& _swapChainDesc,
            const gpu::CommandQueueReference& _presentationQueue);
        void release() override;

        gpu::Texture2DReference getTargetTexture() override;

        void present() override;

    private:
        gpu::SwapChainReference m_swapChain;
    };

    EGO_POINTER(WindowGraphicPresenter);
} // namespace ego
