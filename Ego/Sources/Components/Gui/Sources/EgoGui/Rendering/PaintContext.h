#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "EgoGui/Theme/Theme.h"

#include "DrawData.h"
#include "FontAtlas.h"

namespace ego::gui
{
    class PaintContext final
    {
    public:
        PaintContext(DrawData& _drawData, const Rect& _viewportRect, const FontAtlasPointer& _fontAtlas, const ThemePointer& _theme);

        void pushClipRect(const Rect& _clipRect);
        void popClipRect();

        void drawBox(const Rect& _rect, const NormalizedColorRGBA& _color, ImageID _imageID = InvalidImageID);
        void drawCircle(const Position& _center, float _radius, const NormalizedColorRGBA& _color, ImageID _imageID = InvalidImageID);
        void drawTriangle(
            const Position& _firstPosition,
            const Position& _secondPosition,
            const Position& _thirdPosition,
            const NormalizedColorRGBA& _color,
            ImageID _imageID = InvalidImageID);
        void drawText(std::string_view _text, const Rect& _rect, const NormalizedColorRGBA& _color);

        const Rect& getCurrentClipRect() const;
        const Theme& getTheme() const;

    private:
        void appendQuad(const Rect& _rect, const NormalizedColorRGBA& _color, ImageID _imageID);
        void appendQuad(const Rect& _rect, const Rect& _uvRect, const NormalizedColorRGBA& _color, ImageID _imageID);
        void appendCircle(const Position& _center, float _radius, const NormalizedColorRGBA& _color, ImageID _imageID);
        void appendTriangle(
            const Position& _firstPosition,
            const Position& _secondPosition,
            const Position& _thirdPosition,
            const NormalizedColorRGBA& _color,
            ImageID _imageID);
        void appendDrawCommand(uint32_t _firstIndex, uint32_t _indexCount, ImageID _imageID);

        DrawData& m_drawData;
        FontAtlasPointer m_fontAtlas = nullptr;
        ThemePointer m_theme = nullptr;
        std::vector<Rect> m_clipStack;
    };
} // namespace ego::gui
