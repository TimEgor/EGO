#pragma once

#include <string_view>

#include "EgoCore/Pointer/Pointer.h"

namespace ego::editor
{
    class GuiModalWindow
    {
    public:
        virtual ~GuiModalWindow() = default;

        virtual std::string_view getTitle() const = 0;
        virtual bool draw() = 0;

    protected:
        GuiModalWindow() = default;
    };

    EGO_POINTER(GuiModalWindow);
} // namespace ego::editor
