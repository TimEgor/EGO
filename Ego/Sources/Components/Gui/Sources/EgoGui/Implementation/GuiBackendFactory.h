#pragma once

#include <memory>

#include "GuiBackend.h"

namespace ego::gui
{
    std::unique_ptr<GuiBackend> CreateGuiBackend();
} // namespace ego::gui
