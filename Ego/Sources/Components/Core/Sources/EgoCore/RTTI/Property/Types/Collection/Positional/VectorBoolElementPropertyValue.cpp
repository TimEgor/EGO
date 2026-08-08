#include "EgoCore/RTTI/Property/Types/Collection/Positional/VectorBoolElementPropertyValue.h"

ego::rtti::VectorBoolElementPropertyValue::VectorBoolElementPropertyValue(bool* _value, const TypedScalarPropertyMetaInfo<bool>& _propertyMetaInfo)
    : ScalarPropertyValue(&m_value, _propertyMetaInfo),
      m_value(*_value)
{
}

ego::rtti::VectorBoolElementPropertyValue::VectorBoolElementPropertyValue(const bool* _value, const TypedScalarPropertyMetaInfo<bool>& _propertyMetaInfo)
    : ScalarPropertyValue(static_cast<const void*>(&m_value), _propertyMetaInfo),
      m_value(*_value)
{
}

const ego::rtti::TypedScalarPropertyMetaInfo<bool>& ego::rtti::VectorBoolElementPropertyValue::getMetaInfo() const
{
    return static_cast<const TypedScalarPropertyMetaInfo<bool>&>(PropertyValue::getMetaInfo());
}
