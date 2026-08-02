#pragma once

#include <string>
#include <string_view>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EditorApplication/Gui/Modal/GuiModalWindow.h"

namespace ego::editor
{
    class ErrorWindow final
        : public GuiModalWindow, public NonCopyable
    {
    public:
        explicit ErrorWindow(std::string _message);

    private:
        std::string_view getTitle() const override;
        bool draw() override;

        std::string m_message;
    };

    EGO_POINTER(ErrorWindow);
} // namespace ego::editor
