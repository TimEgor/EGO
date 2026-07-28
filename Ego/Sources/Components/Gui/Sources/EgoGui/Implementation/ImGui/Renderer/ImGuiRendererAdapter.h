#pragma once

#include <cstdint>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/GraphicObjects/Texture.h"
#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

#include "EgoGui/Implementation/ImGui/Renderer/ImGuiFrameTextures.h"
#include "EgoGui/Implementation/ImGui/Renderer/ImGuiTextureStorage.h"
#include "EgoGui/Rendering/GuiRenderData.h"
#include "EgoGui/Rendering/GuiTexture.h"

struct ImDrawCmd;
struct ImDrawData;
struct ImDrawVert;
struct ImTextureData;

namespace ego::gui
{
    class ImGuiPlatformAdapter;

    class ImGuiRendererAdapter final : public NonCopyable
    {
    public:
        bool init();
        void release();

        void beginFrame();
        GuiFrameTextureID bindTexture(const gpu::TextureViewReference& _textureView, TextureSamplingMode _samplingMode);
        bool buildRenderData(const ImGuiPlatformAdapter& _platform, GuiRenderData& _renderData);

    private:
        struct UserData final
        {
            ImGuiTextureStorage m_textureStorage;
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
        ImGuiFrameTextures m_frameTextures;
        bool m_isInitialized = false;
    };
} // namespace ego::gui
