#pragma once

#include <cstddef>

#include "EgoCore/Pointer/MTCountable.h"
#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    class PropertyValue : public MTCountable
    {
    public:
        PropertyValue(void* _object, const PropertyMetaInfo& _propertyMetaInfo);
        PropertyValue(const void* _object, const PropertyMetaInfo& _propertyMetaInfo);
        ~PropertyValue() override;

        EGO_RTTI_VIRTUAL_BASE(PropertyValue);

        const void* getValueAddress() const;
        void* tryGetMutableValueAddress();
        bool isReadOnly() const;
        virtual const PropertyMetaInfo& getMetaInfo() const;

    protected:
        template <typename Value>
        const Value& getValue() const;

        template <typename Value>
        Value* tryGetMutableValue();

    private:
        const void* const m_object;
        const PropertyMetaInfo& m_propertyMetaInfo;
        const bool m_isObjectConst;
    };

    EGO_INTRUSIVE_POINTER(PropertyValue)
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/PropertyValue.hpp"
