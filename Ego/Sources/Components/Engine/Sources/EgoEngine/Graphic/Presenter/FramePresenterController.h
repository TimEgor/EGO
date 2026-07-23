#pragma once

#include <unordered_set>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/Reference/PointerIdentity.h"

#include "EgoGraphicHardware/GraphicObjects/CommandList.h"
#include "EgoGraphicHardware/GraphicObjects/CommandQueue.h"
#include "EgoGraphicHardware/GraphicObjects/Fence.h"
#include "EgoGraphicHardware/GraphicObjects/Texture.h"
#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

namespace ego::engine
{
    class FramePresenterController final : public NonCopyable
    {
    public:
        FramePresenterController() = default;
        ~FramePresenterController() override;

        bool init(bool _enablePresentation);
        void release();
        void clearResources();

        void prepareFrame(const std::vector<GraphicPresenterPointer>& _graphicPresenters);
        void presentFrame();

        gpu::Texture2DReference getTargetTexture(const GraphicPresenterPointer& _graphicPresenter) const;
        bool containsPresenter(const GraphicPresenterPointer& _graphicPresenter) const;

    private:
        using GraphicPresenterCollection = std::unordered_set<GraphicPresenterPointer, PointerIdentityHash, PointerIdentityEqual>;
        using TargetViewCollection = std::vector<gpu::TextureViewReference>;

        bool initPresentation();
        void releasePresentation();
        void clearPresenters();
        bool recordTargetClear(const gpu::Texture2DReference& _targetTexture);
        void transitionPresenterTargets();
        void waitCommandList();
        void signalFence();

        gpu::CommandQueueReference m_presentationQueue = nullptr;
        gpu::GraphicCommandListReference m_commandList = nullptr;
        gpu::FenceReference m_fence = nullptr;
        gpu::Fence::FenceValue m_fenceValue = 0;
        TargetViewCollection m_targetViews;
        GraphicPresenterCollection m_graphicPresenters;
        bool m_isPresentationEnabled = false;
    };
} // namespace ego::engine
