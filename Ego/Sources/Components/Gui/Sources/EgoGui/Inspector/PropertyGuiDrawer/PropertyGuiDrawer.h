#pragma once

#include <functional>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"
#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::gui
{
    using PropertyGuiDrawFunction = std::function<void(const char* _name, void* _value, const rtti::PropertyMetaInfo& _propertyMetaInfo)>;

    class PropertyGuiDrawer : public NonCopyable
    {
    public:
        virtual void draw(const char* _name, void* _value, const rtti::PropertyMetaInfo& _propertyMetaInfo, const PropertyGuiDrawFunction& _drawProperty)
            const = 0;
    };

    EGO_POINTER(PropertyGuiDrawer);
} // namespace ego::gui
