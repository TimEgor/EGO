#include "EgoGui/Inspector/PropertyInspector.h"

#include "EgoCore/RTTI/Property/PropertyMetaInfoCast.h"
#include "EgoCore/RTTI/Property/Types/Collection/Associative/AssociativeCollectionPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Collection/Positional/PositionalCollectionPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Enum/EnumPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/String/StringPropertyMetaInfo.h"

#include "EgoGui/Inspector/PropertyGuiDrawer/AssociativeCollectionPropertyGuiDrawer.h"
#include "EgoGui/Inspector/PropertyGuiDrawer/EnumPropertyGuiDrawer.h"
#include "EgoGui/Inspector/PropertyGuiDrawer/PositionalCollectionPropertyGuiDrawer.h"
#include "EgoGui/Inspector/PropertyGuiDrawer/ScalarPropertyGuiDrawer.h"
#include "EgoGui/Inspector/PropertyGuiDrawer/StringPropertyGuiDrawer.h"

ego::gui::PropertyInspector::PropertyInspector()
    : m_scalarPropertyGuiDrawer(MakePointer<ScalarPropertyGuiDrawer>()),
      m_enumPropertyGuiDrawer(MakePointer<EnumPropertyGuiDrawer>()),
      m_stringPropertyGuiDrawer(MakePointer<StringPropertyGuiDrawer>()),
      m_associativeCollectionPropertyGuiDrawer(MakePointer<AssociativeCollectionPropertyGuiDrawer>()),
      m_positionalCollectionPropertyGuiDrawer(MakePointer<PositionalCollectionPropertyGuiDrawer>())
{
}

ego::gui::PropertyInspector::~PropertyInspector() = default;

ego::gui::PropertyGuiDrawerPointer ego::gui::PropertyInspector::getPropertyGuiDrawerPointer(rtti::TypeMetaInfoID _propertyMetaInfoID) const
{
    return ScalarPropertyGuiDrawer::IsSupported(_propertyMetaInfoID) ? m_scalarPropertyGuiDrawer : nullptr;
}

ego::gui::PropertyGuiDrawerPointer ego::gui::PropertyInspector::getPropertyGuiDrawerPointer(const rtti::PropertyMetaInfo& _propertyMetaInfo) const
{
    const PropertyGuiDrawerPointer propertyGuiDrawer = getPropertyGuiDrawerPointer(_propertyMetaInfo.getObjectTypeMetaInfoID());
    if (propertyGuiDrawer)
    {
        return propertyGuiDrawer;
    }

    if (rtti::CastPropertyMetaInfo<rtti::EnumPropertyMetaInfoBase>(&_propertyMetaInfo))
    {
        return m_enumPropertyGuiDrawer;
    }

    if (rtti::CastPropertyMetaInfo<rtti::StringPropertyMetaInfo>(&_propertyMetaInfo))
    {
        return m_stringPropertyGuiDrawer;
    }

    if (rtti::CastPropertyMetaInfo<rtti::PositionalCollectionPropertyMetaInfo>(&_propertyMetaInfo))
    {
        return m_positionalCollectionPropertyGuiDrawer;
    }

    if (rtti::CastPropertyMetaInfo<rtti::AssociativeCollectionPropertyMetaInfo>(&_propertyMetaInfo))
    {
        return m_associativeCollectionPropertyGuiDrawer;
    }

    return nullptr;
}
