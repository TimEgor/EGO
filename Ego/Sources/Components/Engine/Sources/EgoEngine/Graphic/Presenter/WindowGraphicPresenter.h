#pragma once

#include "EgoEngine/Graphic/RenderHardware/GraphicDevice.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/CommandQueue.h"
#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/SwapChain.h"
#include "EgoEngine/Graphic/RenderHardware/PresentationSurface.h"

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
