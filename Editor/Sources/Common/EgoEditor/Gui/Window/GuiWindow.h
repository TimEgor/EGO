#pragma once

#include <string_view>

#include "EgoCore/Pointer/Pointer.h"

namespace ego::editor
{
    class GuiWindow
    {
    public:
        GuiWindow() = default;
        virtual ~GuiWindow() = default;

        virtual std::string_view getTitle() const = 0;
        bool isVisible() const;
        void setVisible(bool _isVisible);
        void draw();

    private:
        virtual void drawWindow(bool& _isVisible) = 0;

        bool m_isVisible = true;
    };

    EGO_POINTER(GuiWindow);
} // namespace ego::editor
