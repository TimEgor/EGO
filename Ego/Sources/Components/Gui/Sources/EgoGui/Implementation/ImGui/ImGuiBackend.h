#pragma once

#include "EgoGui/Implementation/GuiBackend.h"
#include "EgoGui/Implementation/ImGui/Platform/ImGuiPlatformAdapter.h"
#include "EgoGui/Implementation/ImGui/Renderer/ImGuiRendererAdapter.h"
#include "EgoGui/Rendering/GuiRenderData.h"
#include "EgoGui/Viewport/ViewportProvider.h"

struct ImGuiContext;

namespace ego::gui
{
    class ImGuiBackend final : public GuiBackend
    {
    public:
        ImGuiBackend();
        ~ImGuiBackend() override;

        bool init(const ViewportProviderPointer& _viewportProvider, bool _enableMultiViewport) override;
        bool release() override;

        bool beginFrame(float _deltaTime) override;
        bool endFrame(GuiRenderData& _renderData) override;
        void cancelFrame() override;

        GuiFrameTextureID bindTexture(const gpu::TextureViewPointer& _textureView, TextureSamplingMode _samplingMode) override;

        bool isInitialized() const override;

    private:
        bool initializeContext(bool _enableMultiViewport);

        bool releaseContext();
        void resetState();

        void restorePreviousContext();

        ImGuiRendererAdapter m_rendererAdapter;
        ImGuiPlatformAdapter m_platformAdapter;

        ImGuiContext* m_context = nullptr;
        ImGuiContext* m_previousContext = nullptr;

        bool m_isFrameActive = false;
        bool m_isInitialized = false;
    };
} // namespace ego::gui
