#pragma once

#include <string>

#include "EgoGui/Widgets/Column.h"

namespace ego::gui
{
    ColumnPointer Labeled(std::string _label, const WidgetPointer& _content);
} // namespace ego::gui
