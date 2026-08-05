#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGui/Inspector/PropertyGuiDrawer/PropertyGuiDrawer.h"

namespace ego::gui
{
    class PropertyInspector final : public NonCopyable
    {
    public:
        PropertyInspector();
        ~PropertyInspector() override;

        PropertyGuiDrawerPointer getPropertyGuiDrawerPointer(rtti::TypeMetaInfoID _propertyMetaInfoID) const;
        PropertyGuiDrawerPointer getPropertyGuiDrawerPointer(const rtti::PropertyMetaInfo& _propertyMetaInfo) const;

    private:
        PropertyGuiDrawerPointer m_scalarPropertyGuiDrawer;
        PropertyGuiDrawerPointer m_enumPropertyGuiDrawer;
        PropertyGuiDrawerPointer m_stringPropertyGuiDrawer;
        PropertyGuiDrawerPointer m_associativeCollectionPropertyGuiDrawer;
        PropertyGuiDrawerPointer m_positionalCollectionPropertyGuiDrawer;
    };

    EGO_POINTER(PropertyInspector);
} // namespace ego::gui
