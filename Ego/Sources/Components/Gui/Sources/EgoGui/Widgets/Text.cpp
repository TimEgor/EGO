#include "Text.h"

#include <utility>

#include "EgoGui/Rendering/FontAtlas.h"
#include "EgoGui/Theme/Theme.h"

ego::gui::TextPointer ego::gui::Text::Create()
{
    return new Text();
}

ego::gui::TextPointer ego::gui::Text::Create(std::string _text)
{
    const TextPointer text = Create();
    text->setText(std::move(_text));
    return text;
}

void ego::gui::Text::setText(std::string _text)
{
    if (m_text == _text)
    {
        return;
    }

    m_text = std::move(_text);
    invalidateLayout();
}

const std::string& ego::gui::Text::getText() const
{
    return m_text;
}

ego::gui::Size ego::gui::Text::calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints&)
{
    return _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_text) : SizeZero;
}

void ego::gui::Text::drawBaseLayer(PaintContext& _context) const
{
    _context.drawText(m_text, getLayoutBounds(), _context.getTheme().m_typography.m_primary);
}
