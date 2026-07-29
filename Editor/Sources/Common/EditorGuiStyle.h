#pragma once

#include "EgoGui/GuiStyle.h"

namespace ego::editor
{
    class EditorGuiStyle final : public gui::GuiStyle
    {
    public:
        void apply() override;
    };
} // namespace ego::editor
