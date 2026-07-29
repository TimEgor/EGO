#pragma once

#include <functional>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

#include "EgoGui/GuiStyle.h"
#include "EgoGui/Rendering/GuiRenderData.h"
#include "EgoGui/Rendering/GuiTexture.h"
#include "EgoGui/Viewport/ViewportProvider.h"

namespace ego::gui
{
    class GuiBackend : public NonCopyable
    {
    public:
        using DrawFunction = std::function<bool()>;

        ~GuiBackend() override = default;

        virtual bool init(const ViewportProviderPointer& _viewportProvider, bool _enableMultiViewport) = 0;
        virtual bool release() = 0;

        virtual bool setStyle(const GuiStylePointer& _style) = 0;
        virtual bool update(float _deltaTime, const DrawFunction& _drawFunction, GuiRenderData& _renderData) = 0;

        virtual GuiFrameTextureID bindTexture(const gpu::TextureViewPointer& _textureView, TextureSamplingMode _samplingMode) = 0;

        virtual bool isInitialized() const = 0;
    };
} // namespace ego::gui
