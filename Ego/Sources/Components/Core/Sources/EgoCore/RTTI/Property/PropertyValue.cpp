#include "EgoCore/RTTI/Property/PropertyValue.h"

#include <cstdint>

#include "EgoCore/Assert/Assert.h"

ego::rtti::PropertyValue::PropertyValue(void* _object, const PropertyMetaInfo& _propertyMetaInfo)
    : m_object(_object),
      m_propertyMetaInfo(_propertyMetaInfo),
      m_isObjectConst(false)
{
    EGO_ASSERT(_object);
}

ego::rtti::PropertyValue::PropertyValue(const void* _object, const PropertyMetaInfo& _propertyMetaInfo)
    : m_object(_object),
      m_propertyMetaInfo(_propertyMetaInfo),
      m_isObjectConst(true)
{
    EGO_ASSERT(_object);
}

ego::rtti::PropertyValue::~PropertyValue() = default;

const void* ego::rtti::PropertyValue::getValueAddress() const
{
    return static_cast<const uint8_t*>(m_object) + m_propertyMetaInfo.getOffset();
}

void* ego::rtti::PropertyValue::tryGetMutableValueAddress()
{
    if (isReadOnly())
    {
        return nullptr;
    }

    return const_cast<uint8_t*>(static_cast<const uint8_t*>(m_object)) + m_propertyMetaInfo.getOffset();
}

bool ego::rtti::PropertyValue::isReadOnly() const
{
    return m_isObjectConst || m_propertyMetaInfo.isConst();
}

const ego::rtti::PropertyMetaInfo& ego::rtti::PropertyValue::getMetaInfo() const
{
    return m_propertyMetaInfo;
}
