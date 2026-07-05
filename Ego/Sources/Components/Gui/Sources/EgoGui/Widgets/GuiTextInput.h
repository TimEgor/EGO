#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "EgoGui/GuiWidget.h"

namespace ego::gui
{
    class GuiTextInput;
    EGO_POINTER(GuiTextInput);

    class GuiTextInput final : public GuiWidget
    {
    public:
        GuiTextInput() = default;

        static GuiTextInputPointer Create();

        void setName(std::string _name);
        const std::string& getName() const;

        void setText(std::string _text);
        const std::string& getText() const;

        void setPlaceholder(std::string _placeholder);
        const std::string& getPlaceholder() const;

        void setFocused(bool _isFocused);
        bool isFocused() const;

        GuiReply handleEvent(const GuiInputEvent& _event) override;

        EGO_RTTI_VIRTUAL(GuiTextInput, GuiWidget);

    protected:
        GuiSize onMeasure(const GuiLayoutContext& _context, const GuiSize& _availableSize) override;
        void onArrange(const GuiLayoutContext& _context, const GuiRect& _rect) override;
        void onPaint(GuiPaintContext& _context) const override;

    private:
        using TextIndex = size_t;

        bool handleKeyDown(uint32_t _key);
        bool handleTextInput(uint32_t _codepoint);
        void insertCharacter(char _character);
        void deletePreviousCharacter();
        void deleteNextCharacter();
        bool deleteSelection();
        void selectAll();

        GuiRect getTextRect() const;
        void rebuildCaretOffsets(const GuiFontAtlasPointer& _fontAtlas);
        TextIndex getCaretIndexAtPosition(const GuiPosition& _position) const;
        float getCaretOffset(TextIndex _index) const;
        TextIndex getSelectionBegin() const;
        TextIndex getSelectionEnd() const;
        bool hasSelection() const;
        void clearSelection();
        void setCaretIndex(TextIndex _index, bool _keepSelection);
        void moveCaretLeft(bool _keepSelection);
        void moveCaretRight(bool _keepSelection);
        void moveCaretToStart(bool _keepSelection);
        void moveCaretToEnd(bool _keepSelection);

        std::string m_name;
        std::string m_text;
        std::string m_placeholder;
        GuiRect m_nameRect;
        GuiRect m_inputRect;
        std::vector<float> m_caretOffsets;
        TextIndex m_caretIndex = 0;
        TextIndex m_selectionAnchor = 0;
        bool m_isFocused = false;
        bool m_isHovered = false;
        bool m_isPressed = false;
        bool m_isSelecting = false;
        bool m_isShiftPressed = false;
        bool m_isControlPressed = false;
    };
} // namespace ego::gui
