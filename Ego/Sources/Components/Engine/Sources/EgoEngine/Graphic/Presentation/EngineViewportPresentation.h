#pragma once

#include <array>
#include <cstdint>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGraphicHardware/GraphicObjects/CommandList.h"
#include "EgoGraphicHardware/GraphicObjects/CommandQueue.h"
#include "EgoGraphicHardware/GraphicObjects/Fence.h"
#include "EgoGraphicHardware/GraphicObjects/ResourceView.h"
#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Viewport/ViewportTypes.h"

#include "EgoEngine/Gui/Viewport/EngineGuiViewportBackend.h"

namespace ego::gui
{
    class GuiRender;
} // namespace ego::gui

namespace ego::engine
{
    class EngineViewportPresentation final : public NonCopyable
    {
    public:
        EngineViewportPresentation() = default;
        ~EngineViewportPresentation() override;

        bool init(const EngineViewportHostPointer& _host);
        void release();

        EngineViewportPrepareResult prepare();
        bool resizeTarget();
        bool present(const SharedPointer<gui::GuiRender>& _guiRender, gui::ViewportID _viewportID, const gpu::Texture2DReference& _sceneTexture);
        void wait();

        gpu::Texture2DReference getTargetTexture() const;
        uint32_t getFrameIndex() const;
        bool matchesHost(const EngineViewportHostPointer& _host) const;
        bool isInitialized() const;

    private:
        static constexpr uint32_t FrameResourceCount = 2;
        static constexpr uint32_t InvalidFrameIndex = FrameResourceCount;

        struct FrameResources final
        {
            GraphicPresenterPointer m_presenter = nullptr;
            gpu::Texture2DReference m_target = nullptr;
            gpu::Texture2DReference m_sceneTexture = nullptr;
            gpu::TextureViewReference m_targetView = nullptr;
            gpu::GraphicCommandListReference m_commandList = nullptr;
            gpu::Fence::FenceValue m_fenceValue = 0;
        };

        using FrameResourceCollection = std::array<FrameResources, FrameResourceCount>;

        FrameResources& getCurrentFrameResources();
        const FrameResources& getCurrentFrameResources() const;
        void waitFrame(FrameResources& _resources);
        void clearFrameResources(FrameResources& _resources);
        bool acquireTarget(FrameResources& _resources);
        bool prepareTargetView(FrameResources& _resources);
        bool prepareSceneTexture(FrameResources& _resources, const gpu::Texture2DReference& _sceneTexture);
        void recordSceneCopy(FrameResources& _resources);
        void recordClear(FrameResources& _resources);
        void signalFrameFence(FrameResources& _resources);

        EngineViewportHostPointer m_host = nullptr;
        gpu::CommandQueueReference m_commandQueue = nullptr;
        gpu::FenceReference m_frameFence = nullptr;
        gpu::Fence::FenceValue m_frameFenceValue = 0;
        FrameResourceCollection m_frameResources;
        uint32_t m_currentFrameIndex = InvalidFrameIndex;
        bool m_isInitialized = false;
    };
} // namespace ego::engine
