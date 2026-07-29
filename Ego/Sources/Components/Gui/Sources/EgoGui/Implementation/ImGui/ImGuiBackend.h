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

        bool setStyle(const GuiStylePointer& _style) override;
        bool update(float _deltaTime, const DrawFunction& _drawFunction, GuiRenderData& _renderData) override;

        GuiFrameTextureID bindTexture(const gpu::TextureViewPointer& _textureView, TextureSamplingMode _samplingMode) override;

        bool isInitialized() const override;

    private:
        bool initializeContext(bool _enableMultiViewport);

        bool releaseContext();
        void resetState();

        bool beginFrame(float _deltaTime);
        bool endFrame(GuiRenderData& _renderData);
        void cancelFrame();

        void applyStyle();

        ImGuiRendererAdapter m_rendererAdapter;
        ImGuiPlatformAdapter m_platformAdapter;

        GuiStylePointer m_style = nullptr;
        ImGuiContext* m_context = nullptr;
        ImGuiContext* m_styleContext = nullptr;
        GuiStyleChangeID m_appliedStyleChangeID = InvalidGuiStyleChangeID;

        bool m_isFrameActive = false;
        bool m_isInitialized = false;
    };
} // namespace ego::gui
