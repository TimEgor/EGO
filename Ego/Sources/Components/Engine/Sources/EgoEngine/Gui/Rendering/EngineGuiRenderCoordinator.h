#pragma once

#include <unordered_map>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/GraphicObjects/CommandList.h"
#include "EgoGraphicHardware/GraphicObjects/CommandQueue.h"
#include "EgoGraphicHardware/GraphicObjects/Fence.h"
#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Rendering/GuiFrame.h"

#include "EgoGuiRender/GuiRender.h"

#include "EgoEngine/Gui/Viewport/EngineGuiViewportBackend.h"

namespace ego::render
{
    class Render;
} // namespace ego::render

namespace ego::engine
{
    class EngineGuiRenderCoordinator final : public NonCopyable
    {
    public:
        EngineGuiRenderCoordinator() = default;
        ~EngineGuiRenderCoordinator() override;

        bool init(const EngineGuiViewportBackendPointer& _viewportBackend, const gui::GuiRenderPointer& _guiRender);
        void release();
        void clearResources();

        bool prepare(gui::GuiFrame&& _frame);
        bool renderAndPresent(render::Render& _sceneRender);
        void wait();

        bool isInitialized() const;

    private:
        struct PresentedViewport final
        {
            EngineViewportHostPointer m_host = nullptr;
            GraphicPresenterPointer m_presenter = nullptr;
            gpu::Texture2DReference m_target = nullptr;
            gpu::TextureViewReference m_targetView = nullptr;
            gpu::GraphicCommandListReference m_commandList = nullptr;
        };

        using CommandListMap = std::unordered_map<gui::GuiViewportID, gpu::GraphicCommandListReference>;

        bool prepareViewport(gui::GuiViewportFrame&& _viewportFrame, const gui::GuiFontAtlasPointer& _fontAtlas);
        bool recordViewport(
            gui::GuiViewportID _viewportID,
            const EngineViewportHostPointer& _host,
            const GraphicPresenterPointer& _presenter,
            const gpu::Texture2DReference& _target,
            bool _hasScene,
            std::vector<gpu::CommandListReference>& _commandLists);
        gpu::GraphicCommandListReference getOrCreateCommandList(gui::GuiViewportID _viewportID);
        void removeUnusedViewportResources(const gui::GuiViewportIDCollection& _viewportIDs);
        void signalFrameFence();

        EngineGuiViewportBackendPointer m_viewportBackend = nullptr;
        gui::GuiRenderPointer m_guiRender = nullptr;
        gpu::CommandQueueReference m_commandQueue = nullptr;
        gpu::FenceReference m_frameFence = nullptr;
        gpu::Fence::FenceValue m_frameFenceValue = 0;
        CommandListMap m_commandLists;
        gui::GuiViewportIDCollection m_knownViewportIDs;
        gui::GuiViewportIDCollection m_preparedViewportIDs;
        std::vector<PresentedViewport> m_presentedViewports;
        gui::GuiViewportID m_primaryViewportID = gui::InvalidGuiViewportID;
        bool m_isInitialized = false;
    };
} // namespace ego::engine
