#pragma once

#include <string_view>
#include <vector>

#include "GuiDrawData.h"
#include "GuiFontAtlas.h"

namespace ego::gui
{
    class GuiPaintContext final
    {
    public:
        GuiPaintContext(GuiDrawData& _drawData, const GuiRect& _viewportRect, const GuiFontAtlasPointer& _fontAtlas);

        void pushClipRect(const GuiRect& _clipRect);
        void popClipRect();

        void drawBox(const GuiRect& _rect, const GuiColor& _color, GuiTextureID _textureId = InvalidGuiTextureID);
        void drawCircle(const GuiPosition& _center, float _radius, const GuiColor& _color, GuiTextureID _textureId = InvalidGuiTextureID);
        void drawTriangle(
            const GuiPosition& _firstPosition,
            const GuiPosition& _secondPosition,
            const GuiPosition& _thirdPosition,
            const GuiColor& _color,
            GuiTextureID _textureId = InvalidGuiTextureID);
        void drawText(std::string_view _text, const GuiRect& _rect, const GuiColor& _color);

        const GuiRect& getCurrentClipRect() const;
    private:
        void appendQuad(const GuiRect& _rect, const GuiColor& _color, GuiTextureID _textureId);
        void appendQuad(const GuiRect& _rect, const GuiRect& _uvRect, const GuiColor& _color, GuiTextureID _textureId);
        void appendCircle(const GuiPosition& _center, float _radius, const GuiColor& _color, GuiTextureID _textureId);
        void appendTriangle(
            const GuiPosition& _firstPosition,
            const GuiPosition& _secondPosition,
            const GuiPosition& _thirdPosition,
            const GuiColor& _color,
            GuiTextureID _textureId);

        GuiDrawData& m_drawData;
        GuiFontAtlasPointer m_fontAtlas = nullptr;
        std::vector<GuiRect> m_clipStack;
    };
} // namespace ego::gui
