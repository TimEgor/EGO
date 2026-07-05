#include "GuiTextInput.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "EgoGui/GuiFontAtlas.h"

namespace
{
    constexpr uint32_t KeyBackspace = 8;
    constexpr uint32_t KeyTab = 9;
    constexpr uint32_t KeyEnter = 13;
    constexpr uint32_t KeyShift = 16;
    constexpr uint32_t KeyControl = 17;
    constexpr uint32_t KeyEscape = 27;
    constexpr uint32_t KeyEnd = 35;
    constexpr uint32_t KeyHome = 36;
    constexpr uint32_t KeyLeft = 37;
    constexpr uint32_t KeyRight = 39;
    constexpr uint32_t KeyDelete = 46;
    constexpr uint32_t KeySpace = 32;
    constexpr uint32_t Key0 = 48;
    constexpr uint32_t Key9 = 57;
    constexpr uint32_t KeyA = 65;
    constexpr uint32_t KeyZ = 90;
    constexpr uint32_t KeyNumpad0 = 96;
    constexpr uint32_t KeyNumpad9 = 105;
    constexpr uint32_t KeyLeftShift = 160;
    constexpr uint32_t KeyRightShift = 161;
    constexpr uint32_t KeyLeftControl = 162;
    constexpr uint32_t KeyRightControl = 163;
    constexpr float NameSpacing = 4.0f;
    constexpr float FieldHeight = 26.0f;
    constexpr float FieldMinWidth = 160.0f;
    constexpr float TextPaddingX = 6.0f;
    constexpr float TextPaddingY = 5.0f;

    bool IsShiftKey(uint32_t _key)
    {
        return _key == KeyShift || _key == KeyLeftShift || _key == KeyRightShift;
    }

    bool IsControlKey(uint32_t _key)
    {
        return _key == KeyControl || _key == KeyLeftControl || _key == KeyRightControl;
    }

    bool TryConvertKeyToCharacter(uint32_t _key, bool _isShiftPressed, char& _character)
    {
        if (_key >= KeyA && _key <= KeyZ)
        {
            const char character = static_cast<char>('a' + (_key - KeyA));
            _character = _isShiftPressed ? static_cast<char>(character - 'a' + 'A') : character;
            return true;
        }

        if (_key >= Key0 && _key <= Key9)
        {
            static constexpr char ShiftDigits[] = {
                ')',
                '!',
                '@',
                '#',
                '$',
                '%',
                '^',
                '&',
                '*',
                '(',
            };
            const uint32_t digitIndex = _key - Key0;
            _character = _isShiftPressed ? ShiftDigits[digitIndex] : static_cast<char>('0' + digitIndex);
            return true;
        }

        if (_key >= KeyNumpad0 && _key <= KeyNumpad9)
        {
            _character = static_cast<char>('0' + (_key - KeyNumpad0));
            return true;
        }

        if (_key == KeySpace)
        {
            _character = ' ';
            return true;
        }

        return false;
    }

    bool TryConvertCodepointToCharacter(uint32_t _codepoint, char& _character)
    {
        if (_codepoint < 32 || _codepoint > 126)
        {
            return false;
        }

        _character = static_cast<char>(_codepoint);
        return true;
    }
} // namespace

ego::gui::GuiTextInputPointer ego::gui::GuiTextInput::Create()
{
    return new GuiTextInput();
}

void ego::gui::GuiTextInput::setName(std::string _name)
{
    m_name = std::move(_name);
}

const std::string& ego::gui::GuiTextInput::getName() const
{
    return m_name;
}

void ego::gui::GuiTextInput::setText(std::string _text)
{
    m_text = std::move(_text);
    m_caretIndex = m_text.size();
    clearSelection();
}

const std::string& ego::gui::GuiTextInput::getText() const
{
    return m_text;
}

void ego::gui::GuiTextInput::setPlaceholder(std::string _placeholder)
{
    m_placeholder = std::move(_placeholder);
}

const std::string& ego::gui::GuiTextInput::getPlaceholder() const
{
    return m_placeholder;
}

void ego::gui::GuiTextInput::setFocused(bool _isFocused)
{
    m_isFocused = _isFocused;
    if (!m_isFocused)
    {
        m_isPressed = false;
        m_isSelecting = false;
        m_isShiftPressed = false;
        m_isControlPressed = false;
        clearSelection();
    }
}

bool ego::gui::GuiTextInput::isFocused() const
{
    return m_isFocused;
}

ego::gui::GuiReply ego::gui::GuiTextInput::handleEvent(const GuiInputEvent& _event)
{
    const bool containsMouse = _event.m_hasPosition && m_inputRect.contains(_event.m_position);
    if (_event.m_type == GuiInputEventType::MouseMove)
    {
        m_isHovered = containsMouse;
        if (m_isSelecting && _event.m_hasPosition)
        {
            setCaretIndex(getCaretIndexAtPosition(_event.m_position), true);
            return GuiReply::Handled();
        }

        return GuiReply::Unhandled();
    }

    if (_event.m_type == GuiInputEventType::MouseButtonDown && _event.m_mouseButton == GuiMouseButton::Left)
    {
        setFocused(containsMouse);
        m_isHovered = containsMouse;
        m_isPressed = containsMouse;
        m_isSelecting = containsMouse;
        if (containsMouse)
        {
            setCaretIndex(getCaretIndexAtPosition(_event.m_position), m_isShiftPressed);
        }

        return containsMouse ? GuiReply::Handled() : GuiReply::Unhandled();
    }

    if (_event.m_type == GuiInputEventType::MouseButtonUp && _event.m_mouseButton == GuiMouseButton::Left && m_isPressed)
    {
        m_isPressed = false;
        m_isSelecting = false;
        m_isHovered = containsMouse;
        return GuiReply::Handled();
    }

    if (_event.m_type == GuiInputEventType::KeyDown && m_isFocused)
    {
        return handleKeyDown(_event.m_key) ? GuiReply::Handled() : GuiReply::Unhandled();
    }

    if (_event.m_type == GuiInputEventType::KeyUp && IsShiftKey(_event.m_key))
    {
        m_isShiftPressed = false;
        return m_isFocused ? GuiReply::Handled() : GuiReply::Unhandled();
    }

    if (_event.m_type == GuiInputEventType::KeyUp && IsControlKey(_event.m_key))
    {
        m_isControlPressed = false;
        return m_isFocused ? GuiReply::Handled() : GuiReply::Unhandled();
    }

    if (_event.m_type == GuiInputEventType::TextInput && m_isFocused)
    {
        return handleTextInput(_event.m_textCodepoint) ? GuiReply::Handled() : GuiReply::Unhandled();
    }

    return GuiReply::Unhandled();
}

ego::gui::GuiSize ego::gui::GuiTextInput::onMeasure(const GuiLayoutContext& _context, const GuiSize&)
{
    GuiSize nameSize = GuiSizeZero;
    if (_context.m_fontAtlas && !m_name.empty())
    {
        nameSize = _context.m_fontAtlas->measureText(m_name);
    }

    const std::string& displayText = m_text.empty() ? m_placeholder : m_text;
    GuiSize textSize = GuiSizeZero;
    if (_context.m_fontAtlas)
    {
        textSize = _context.m_fontAtlas->measureText(displayText);
    }

    const float nameHeight = m_name.empty() ? 0.0f : nameSize.m_y + NameSpacing;
    const float width = (std::max)(
        FieldMinWidth,
        (std::max)(nameSize.m_x, textSize.m_x + TextPaddingX * 2.0f));
    return GuiSize(width, nameHeight + FieldHeight);
}

void ego::gui::GuiTextInput::onArrange(const GuiLayoutContext& _context, const GuiRect& _rect)
{
    rebuildCaretOffsets(_context.m_fontAtlas);

    float currentY = _rect.m_position.m_y;
    if (!m_name.empty())
    {
        const GuiSize nameSize = _context.m_fontAtlas ? _context.m_fontAtlas->measureText(m_name) : GuiSizeZero;
        m_nameRect = GuiRect(_rect.m_position.m_x, currentY, _rect.m_size.m_x, nameSize.m_y);
        currentY += nameSize.m_y + NameSpacing;
    }
    else
    {
        m_nameRect = GuiRect(_rect.m_position, GuiSizeZero);
    }

    m_inputRect = GuiRect(_rect.m_position.m_x, currentY, _rect.m_size.m_x, FieldHeight);
}

void ego::gui::GuiTextInput::onPaint(GuiPaintContext& _context) const
{
    if (!m_name.empty())
    {
        _context.drawText(m_name, m_nameRect, GuiColorWhite);
    }

    GuiColor borderColor = GuiColor(0.12f, 0.13f, 0.15f, 1.0f);
    if (m_isHovered)
    {
        borderColor = GuiColor(0.20f, 0.23f, 0.26f, 1.0f);
    }
    if (m_isFocused)
    {
        borderColor = GuiColor(0.20f, 0.55f, 0.95f, 1.0f);
    }

    _context.drawBox(m_inputRect, borderColor);

    const GuiRect contentRect(
        m_inputRect.m_position.m_x + 1.0f,
        m_inputRect.m_position.m_y + 1.0f,
        (std::max)(0.0f, m_inputRect.m_size.m_x - 2.0f),
        (std::max)(0.0f, m_inputRect.m_size.m_y - 2.0f));
    _context.drawBox(contentRect, GuiColor(0.04f, 0.05f, 0.06f, 1.0f));

    const GuiRect textRect = getTextRect();
    if (m_isFocused && hasSelection())
    {
        const float selectionLeft = textRect.m_position.m_x + getCaretOffset(getSelectionBegin());
        const float selectionRight = textRect.m_position.m_x + getCaretOffset(getSelectionEnd());
        const float clippedSelectionLeft = (std::max)(textRect.m_position.m_x, selectionLeft);
        const float clippedSelectionRight = (std::min)(textRect.getRight(), selectionRight);
        if (clippedSelectionRight > clippedSelectionLeft)
        {
            const GuiRect selectionRect(
                clippedSelectionLeft,
                textRect.m_position.m_y,
                clippedSelectionRight - clippedSelectionLeft,
                textRect.m_size.m_y);
            _context.drawBox(selectionRect, GuiColor(0.12f, 0.34f, 0.62f, 1.0f));
        }
    }

    const bool hasText = !m_text.empty();
    if (hasText || !m_isFocused)
    {
        const std::string& displayText = hasText ? m_text : m_placeholder;
        const GuiColor textColor = hasText ? GuiColorWhite : GuiColor(0.55f, 0.58f, 0.62f, 1.0f);
        _context.drawText(displayText, textRect, textColor);
    }

    if (m_isFocused)
    {
        const float caretX = (std::min)(
            textRect.m_position.m_x + getCaretOffset(m_caretIndex) + 1.0f,
            textRect.m_position.m_x + textRect.m_size.m_x);
        const GuiRect caretRect(caretX, textRect.m_position.m_y, 1.0f, textRect.m_size.m_y);
        _context.drawBox(caretRect, GuiColorWhite);
    }
}

bool ego::gui::GuiTextInput::handleKeyDown(uint32_t _key)
{
    if (IsShiftKey(_key))
    {
        m_isShiftPressed = true;
        return true;
    }

    if (IsControlKey(_key))
    {
        m_isControlPressed = true;
        return true;
    }

    if (m_isControlPressed && _key == KeyA)
    {
        selectAll();
        return true;
    }

    if (_key == KeyEscape || _key == KeyEnter)
    {
        setFocused(false);
        return true;
    }

    if (_key == KeyLeft)
    {
        moveCaretLeft(m_isShiftPressed);
        return true;
    }

    if (_key == KeyRight)
    {
        moveCaretRight(m_isShiftPressed);
        return true;
    }

    if (_key == KeyHome)
    {
        moveCaretToStart(m_isShiftPressed);
        return true;
    }

    if (_key == KeyEnd)
    {
        moveCaretToEnd(m_isShiftPressed);
        return true;
    }

    if (_key == KeyBackspace)
    {
        deletePreviousCharacter();
        return true;
    }

    if (_key == KeyDelete)
    {
        deleteNextCharacter();
        return true;
    }

    if (_key == KeyTab || m_isControlPressed)
    {
        return false;
    }

    char character = '\0';
    if (!TryConvertKeyToCharacter(_key, m_isShiftPressed, character))
    {
        return false;
    }

    insertCharacter(character);
    return true;
}

bool ego::gui::GuiTextInput::handleTextInput(uint32_t _codepoint)
{
    char character = '\0';
    if (!TryConvertCodepointToCharacter(_codepoint, character))
    {
        return false;
    }

    insertCharacter(character);
    return true;
}

void ego::gui::GuiTextInput::insertCharacter(char _character)
{
    deleteSelection();
    m_text.insert(m_text.begin() + static_cast<std::ptrdiff_t>(m_caretIndex), _character);
    setCaretIndex(m_caretIndex + 1, false);
}

void ego::gui::GuiTextInput::deletePreviousCharacter()
{
    if (deleteSelection())
    {
        return;
    }

    if (m_caretIndex == 0)
    {
        return;
    }

    const TextIndex removedIndex = m_caretIndex - 1;
    m_text.erase(removedIndex, 1);
    setCaretIndex(removedIndex, false);
}

void ego::gui::GuiTextInput::deleteNextCharacter()
{
    if (deleteSelection())
    {
        return;
    }

    if (m_caretIndex >= m_text.size())
    {
        return;
    }

    m_text.erase(m_caretIndex, 1);
    setCaretIndex(m_caretIndex, false);
}

bool ego::gui::GuiTextInput::deleteSelection()
{
    if (!hasSelection())
    {
        return false;
    }

    const TextIndex selectionBegin = getSelectionBegin();
    const TextIndex selectionEnd = getSelectionEnd();
    m_text.erase(selectionBegin, selectionEnd - selectionBegin);
    setCaretIndex(selectionBegin, false);
    return true;
}

void ego::gui::GuiTextInput::selectAll()
{
    if (!m_text.empty())
    {
        m_selectionAnchor = 0;
        m_caretIndex = m_text.size();
    }
}

ego::gui::GuiRect ego::gui::GuiTextInput::getTextRect() const
{
    return GuiRect(
        m_inputRect.m_position.m_x + 1.0f + TextPaddingX,
        m_inputRect.m_position.m_y + 1.0f + TextPaddingY,
        (std::max)(0.0f, m_inputRect.m_size.m_x - 2.0f - TextPaddingX * 2.0f),
        (std::max)(0.0f, m_inputRect.m_size.m_y - 2.0f - TextPaddingY * 2.0f));
}

void ego::gui::GuiTextInput::rebuildCaretOffsets(const GuiFontAtlasPointer& _fontAtlas)
{
    m_caretOffsets.clear();
    m_caretOffsets.reserve(m_text.size() + 1);
    m_caretOffsets.push_back(0.0f);

    float currentOffset = 0.0f;
    if (_fontAtlas)
    {
        for (const char character : m_text)
        {
            const GuiFontGlyph* glyph = _fontAtlas->getGlyph(character);
            if (glyph)
            {
                currentOffset += glyph->m_advance;
            }

            m_caretOffsets.push_back(currentOffset);
        }
    }
    else
    {
        m_caretOffsets.resize(m_text.size() + 1, 0.0f);
    }

    m_caretIndex = (std::min)(m_caretIndex, m_text.size());
    m_selectionAnchor = (std::min)(m_selectionAnchor, m_text.size());
}

ego::gui::GuiTextInput::TextIndex ego::gui::GuiTextInput::getCaretIndexAtPosition(const GuiPosition& _position) const
{
    const GuiRect textRect = getTextRect();
    const float localX = (std::max)(0.0f, _position.m_x - textRect.m_position.m_x);
    for (TextIndex index = 0; index + 1 < m_caretOffsets.size(); ++index)
    {
        const float left = m_caretOffsets[index];
        const float right = m_caretOffsets[index + 1];
        const float center = left + (right - left) * 0.5f;
        if (localX < center)
        {
            return index;
        }
    }

    return m_text.size();
}

float ego::gui::GuiTextInput::getCaretOffset(TextIndex _index) const
{
    if (m_caretOffsets.empty())
    {
        return 0.0f;
    }

    const TextIndex clampedIndex = (std::min)(_index, m_caretOffsets.size() - 1);
    return m_caretOffsets[clampedIndex];
}

ego::gui::GuiTextInput::TextIndex ego::gui::GuiTextInput::getSelectionBegin() const
{
    return (std::min)(m_caretIndex, m_selectionAnchor);
}

ego::gui::GuiTextInput::TextIndex ego::gui::GuiTextInput::getSelectionEnd() const
{
    return (std::max)(m_caretIndex, m_selectionAnchor);
}

bool ego::gui::GuiTextInput::hasSelection() const
{
    return m_caretIndex != m_selectionAnchor;
}

void ego::gui::GuiTextInput::clearSelection()
{
    m_selectionAnchor = m_caretIndex;
}

void ego::gui::GuiTextInput::setCaretIndex(TextIndex _index, bool _keepSelection)
{
    m_caretIndex = (std::min)(_index, m_text.size());
    if (!_keepSelection)
    {
        clearSelection();
    }
}

void ego::gui::GuiTextInput::moveCaretLeft(bool _keepSelection)
{
    if (!_keepSelection && hasSelection())
    {
        setCaretIndex(getSelectionBegin(), false);
        return;
    }

    if (m_caretIndex > 0)
    {
        setCaretIndex(m_caretIndex - 1, _keepSelection);
    }
    else if (!_keepSelection)
    {
        clearSelection();
    }
}

void ego::gui::GuiTextInput::moveCaretRight(bool _keepSelection)
{
    if (!_keepSelection && hasSelection())
    {
        setCaretIndex(getSelectionEnd(), false);
        return;
    }

    if (m_caretIndex < m_text.size())
    {
        setCaretIndex(m_caretIndex + 1, _keepSelection);
    }
    else if (!_keepSelection)
    {
        clearSelection();
    }
}

void ego::gui::GuiTextInput::moveCaretToStart(bool _keepSelection)
{
    setCaretIndex(0, _keepSelection);
}

void ego::gui::GuiTextInput::moveCaretToEnd(bool _keepSelection)
{
    setCaretIndex(m_text.size(), _keepSelection);
}
