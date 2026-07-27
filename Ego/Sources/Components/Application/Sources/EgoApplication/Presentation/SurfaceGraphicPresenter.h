#pragma once

#include "EgoCore/Platform/Surface/PlatformSurface.h"

#include "EgoGraphicHardware/GraphicDevice.h"
#include "EgoGraphicHardware/GraphicObjects/CommandQueue.h"
#include "EgoGraphicHardware/GraphicObjects/SwapChain.h"
#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

namespace ego::application
{
    class SurfaceGraphicPresenter final : public GraphicPresenter
    {
    public:
        SurfaceGraphicPresenter() = default;
        ~SurfaceGraphicPresenter() override;

        bool init(
            GraphicDevice& _graphicDevice,
            const PlatformSurface& _surface,
            const gpu::SwapChainDesc& _swapChainDesc,
            const gpu::CommandQueueReference& _presentationQueue);
        void release() override;

        bool prepare() override;
        gpu::Texture2DReference getTargetTexture() override;
        bool shouldClearTarget() const override;
        gpu::GraphicResourceState getPresentationState() const override;

        bool resize(const gpu::Texture2DSize& _size);
        void present() override;

    private:
        gpu::SwapChainReference m_swapChain;
        gpu::Texture2DSize m_pendingSize = UInt32Vector2Zero;
    };

    EGO_POINTER(SurfaceGraphicPresenter);
} // namespace ego::application
