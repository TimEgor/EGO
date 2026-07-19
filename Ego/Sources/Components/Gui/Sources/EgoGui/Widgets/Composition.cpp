#include "Composition.h"

#include <utility>

#include "EgoGui/Widgets/Text.h"

ego::gui::ColumnPointer ego::gui::Labeled(std::string _label, const WidgetPointer& _content)
{
    const TextPointer label = Text::Create(std::move(_label));
    if (!label || !_content)
    {
        return nullptr;
    }

    return Column::Create({label, _content});
}
