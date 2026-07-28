#pragma once

#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/Rendering/GuiRenderData.h"

namespace ego::gui
{
    class GuiRender;
    class GuiRenderPlugin;

    EGO_POINTER(GuiRenderPlugin);
} // namespace ego::gui

namespace ego::engine
{
    class GuiRenderController final : public NonCopyable
    {
    public:
        using FrameRenderTargetCollection = std::vector<gpu::Texture2DPointer>;

        GuiRenderController() = default;
        ~GuiRenderController() override;

        bool init(const gui::GuiRenderPluginPointer& _guiRenderPlugin);
        void release();

        void clearResources();

        bool prepare(gui::GuiRenderData&& _renderData);
        bool renderFrame(const FrameRenderTargetCollection& _targets);

        bool isInitialized() const;

    private:
        gui::GuiRenderPluginPointer m_guiRenderPlugin = nullptr;
        SharedPointer<gui::GuiRender> m_guiRender = nullptr;
    };

    EGO_POINTER(GuiRenderController);
} // namespace ego::engine
