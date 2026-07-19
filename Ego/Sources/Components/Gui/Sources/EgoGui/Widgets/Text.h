#pragma once

#include <string>

#include "EgoGui/Widgets/Widget.h"

namespace ego::gui
{
    class Text;
    EGO_POINTER(Text);

    class Text final : public Widget
    {
    public:
        static TextPointer Create();
        static TextPointer Create(std::string _text);

        void setText(std::string _text);
        const std::string& getText() const;

        EGO_RTTI_VIRTUAL(Text, Widget);

    protected:
        Size calculatePreferredSize(const LayoutContext& _context, const LayoutConstraints& _constraints) override;
        void drawBaseLayer(PaintContext& _context) const override;

    private:
        Text() = default;

        std::string m_text;
    };

} // namespace ego::gui
