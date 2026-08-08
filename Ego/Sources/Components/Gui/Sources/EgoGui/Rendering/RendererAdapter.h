#pragma once

#include <cstdint>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"
#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

#include "EgoGui/Rendering/FrameTextures.h"
#include "EgoGui/Rendering/TextureStorage.h"
#include "EgoGui/Rendering/GuiRenderData.h"
#include "EgoGui/Rendering/GuiTexture.h"

struct ImDrawCmd;
struct ImDrawData;
struct ImDrawVert;
struct ImTextureData;

namespace ego::gui
{
    class PlatformAdapter;

    class RendererAdapter final : public NonCopyable
    {
    public:
        bool init();
        void release();

        void beginFrame();
        GuiFrameTextureID bindTexture(const gpu::TextureViewPointer& _textureView, TextureSamplingMode _samplingMode);
        bool buildRenderData(const PlatformAdapter& _platform, GuiRenderData& _renderData);

    private:
        struct UserData final
        {
            TextureStorage m_textureStorage;
        };

        bool bindImGuiTextures(const ImDrawData& _drawData);

        bool appendViewport(const ImDrawData& _drawData, const GraphicPresenterPointer& _graphicPresenter, GuiRenderData& _renderData);
        static Vertex ConvertVertex(const ImDrawVert& _vertex, float _displayPositionX, float _displayPositionY);
        static TextureSamplingMode GetSamplingMode(const ImTextureData& _textureData);

        bool appendCommand(
            const ImDrawCmd& _source,
            uint32_t _globalIndexOffset,
            uint32_t _globalVertexOffset,
            float _displayPositionX,
            float _displayPositionY,
            TextureFilteringMode _filteringMode,
            GuiRenderData& _renderData,
            DrawData& _drawData) const;

        UserData m_userData;
        FrameTextures m_frameTextures;
        bool m_isInitialized = false;
    };
} // namespace ego::gui
