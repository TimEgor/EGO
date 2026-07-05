#include "GuiRadioGroup.h"

#include <algorithm>
#include <utility>

#include "EgoGui/GuiFontAtlas.h"

namespace
{
    constexpr float ButtonRadius = 7.0f;
    constexpr float ButtonInnerRadius = 4.0f;
    constexpr float ButtonSelectedRadius = 3.0f;
    constexpr float ButtonSize = 16.0f;
    constexpr float TextOffset = 24.0f;
    constexpr float OptionSpacing = 4.0f;
    constexpr float TitleSpacing = 6.0f;

    const std::string EmptyString;
} // namespace

ego::gui::GuiRadioGroupPointer ego::gui::GuiRadioGroup::Create()
{
    return new GuiRadioGroup();
}

void ego::gui::GuiRadioGroup::setTitle(std::string _title)
{
    m_title = std::move(_title);
}

const std::string& ego::gui::GuiRadioGroup::getTitle() const
{
    return m_title;
}

void ego::gui::GuiRadioGroup::addOption(std::string _text)
{
    m_options.push_back(std::move(_text));
    m_optionRects.resize(m_options.size());

    if (!hasSelectedOption())
    {
        m_selectedOptionIndex = 0;
    }
}

void ego::gui::GuiRadioGroup::clearOptions()
{
    m_options.clear();
    m_optionRects.clear();
    m_selectedOptionIndex = InvalidOptionIndex;
    m_hoveredOptionIndex = InvalidOptionIndex;
    m_pressedOptionIndex = InvalidOptionIndex;
}

size_t ego::gui::GuiRadioGroup::getOptionCount() const
{
    return m_options.size();
}

const std::string& ego::gui::GuiRadioGroup::getOptionText(OptionIndex _index) const
{
    return isValidOptionIndex(_index) ? m_options[_index] : EmptyString;
}

void ego::gui::GuiRadioGroup::setSelectedOptionIndex(OptionIndex _index)
{
    m_selectedOptionIndex = isValidOptionIndex(_index) ? _index : InvalidOptionIndex;
}

ego::gui::GuiRadioGroup::OptionIndex ego::gui::GuiRadioGroup::getSelectedOptionIndex() const
{
    return m_selectedOptionIndex;
}

bool ego::gui::GuiRadioGroup::hasSelectedOption() const
{
    return isValidOptionIndex(m_selectedOptionIndex);
}

const std::string& ego::gui::GuiRadioGroup::getSelectedOptionText() const
{
    return hasSelectedOption() ? m_options[m_selectedOptionIndex] : EmptyString;
}

ego::gui::GuiReply ego::gui::GuiRadioGroup::handleEvent(const GuiInputEvent& _event)
{
    if (!isVisible())
    {
        return GuiReply::Unhandled();
    }

    const OptionIndex optionIndex = _event.m_hasPosition ? findOptionAtPosition(_event.m_position) : InvalidOptionIndex;
    if (_event.m_type == GuiInputEventType::MouseMove)
    {
        m_hoveredOptionIndex = optionIndex;
        return GuiReply::Unhandled();
    }

    if (_event.m_type == GuiInputEventType::MouseButtonDown && _event.m_mouseButton == GuiMouseButton::Left && isValidOptionIndex(optionIndex))
    {
        m_hoveredOptionIndex = optionIndex;
        m_pressedOptionIndex = optionIndex;
        return GuiReply::Handled();
    }

    if (_event.m_type == GuiInputEventType::MouseButtonUp && _event.m_mouseButton == GuiMouseButton::Left && isValidOptionIndex(m_pressedOptionIndex))
    {
        if (optionIndex == m_pressedOptionIndex)
        {
            m_selectedOptionIndex = optionIndex;
        }

        m_hoveredOptionIndex = optionIndex;
        m_pressedOptionIndex = InvalidOptionIndex;
        return GuiReply::Handled();
    }

    return GuiReply::Unhandled();
}

ego::gui::GuiSize ego::gui::GuiRadioGroup::onMeasure(const GuiLayoutContext& _context, const GuiSize&)
{
    GuiSize result = GuiSizeZero;

    if (!m_title.empty())
    {
        const GuiSize titleSize = _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_title) : GuiSizeZero;
        result.m_x = (std::max)(result.m_x, titleSize.m_x);
        result.m_y += titleSize.m_y + TitleSpacing;
    }

    for (OptionIndex optionIndex = 0; optionIndex < m_options.size(); ++optionIndex)
    {
        const GuiSize textSize = _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_options[optionIndex]) : GuiSizeZero;
        result.m_x = (std::max)(result.m_x, TextOffset + textSize.m_x);
        result.m_y += (std::max)(ButtonSize, textSize.m_y);

        if (optionIndex + 1 < m_options.size())
        {
            result.m_y += OptionSpacing;
        }
    }

    return result;
}

void ego::gui::GuiRadioGroup::onArrange(const GuiLayoutContext& _context, const GuiRect& _rect)
{
    float currentY = _rect.m_position.m_y;
    if (!m_title.empty())
    {
        const GuiSize titleSize = _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_title) : GuiSizeZero;
        m_titleRect = GuiRect(_rect.m_position.m_x, currentY, _rect.m_size.m_x, titleSize.m_y);
        currentY += titleSize.m_y + TitleSpacing;
    }
    else
    {
        m_titleRect = GuiRect(_rect.m_position, GuiSizeZero);
    }

    m_optionRects.resize(m_options.size());
    for (OptionIndex optionIndex = 0; optionIndex < m_options.size(); ++optionIndex)
    {
        const GuiSize textSize = _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_options[optionIndex]) : GuiSizeZero;
        const float optionHeight = (std::max)(ButtonSize, textSize.m_y);
        m_optionRects[optionIndex] = GuiRect(_rect.m_position.m_x, currentY, _rect.m_size.m_x, optionHeight);
        currentY += optionHeight + OptionSpacing;
    }
}

void ego::gui::GuiRadioGroup::onPaint(GuiPaintContext& _context) const
{
    if (!m_title.empty())
    {
        _context.drawText(m_title, m_titleRect, GuiColorWhite);
    }

    for (OptionIndex optionIndex = 0; optionIndex < m_options.size(); ++optionIndex)
    {
        const GuiRect& optionRect = m_optionRects[optionIndex];
        const GuiPosition center(optionRect.m_position.m_x + ButtonRadius, optionRect.m_position.m_y + ButtonRadius);

        GuiColor outerColor = GuiColor(0.12f, 0.13f, 0.15f, 1.0f);
        if (optionIndex == m_hoveredOptionIndex)
        {
            outerColor = GuiColor(0.18f, 0.21f, 0.24f, 1.0f);
        }
        if (optionIndex == m_pressedOptionIndex)
        {
            outerColor = GuiColor(0.08f, 0.10f, 0.12f, 1.0f);
        }

        _context.drawCircle(center, ButtonRadius, outerColor);
        _context.drawCircle(center, ButtonInnerRadius, GuiColor(0.04f, 0.05f, 0.06f, 1.0f));

        if (optionIndex == m_selectedOptionIndex)
        {
            _context.drawCircle(center, ButtonSelectedRadius, GuiColor(0.20f, 0.55f, 0.95f, 1.0f));
        }

        const GuiRect textRect(optionRect.m_position.m_x + TextOffset, optionRect.m_position.m_y, optionRect.m_size.m_x - TextOffset, optionRect.m_size.m_y);
        _context.drawText(m_options[optionIndex], textRect, GuiColorWhite);
    }
}

ego::gui::GuiRadioGroup::OptionIndex ego::gui::GuiRadioGroup::findOptionAtPosition(const GuiPosition& _position) const
{
    for (OptionIndex optionIndex = 0; optionIndex < m_optionRects.size(); ++optionIndex)
    {
        if (m_optionRects[optionIndex].contains(_position))
        {
            return optionIndex;
        }
    }

    return InvalidOptionIndex;
}

bool ego::gui::GuiRadioGroup::isValidOptionIndex(OptionIndex _index) const
{
    return _index < m_options.size();
}
