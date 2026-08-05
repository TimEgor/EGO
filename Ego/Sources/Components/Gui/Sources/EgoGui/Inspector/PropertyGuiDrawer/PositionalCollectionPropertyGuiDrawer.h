#pragma once

#include "EgoGui/Inspector/PropertyGuiDrawer/PropertyGuiDrawer.h"

namespace ego::gui
{
    class PositionalCollectionPropertyGuiDrawer final : public PropertyGuiDrawer
    {
    public:
        void draw(const char* _name, void* _value, const rtti::PropertyMetaInfo& _propertyMetaInfo, const PropertyGuiDrawFunction& _drawProperty)
            const override;
    };
} // namespace ego::gui
