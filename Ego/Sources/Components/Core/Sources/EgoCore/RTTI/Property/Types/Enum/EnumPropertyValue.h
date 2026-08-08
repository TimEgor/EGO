#pragma once

#include <limits>
#include <type_traits>

#include "EgoCore/RTTI/Property/PropertyValue.h"
#include "EgoCore/RTTI/Property/Types/Enum/EnumPropertyMetaInfo.h"

namespace ego::rtti
{
    inline constexpr size_t InvalidEnumValueIndex = std::numeric_limits<size_t>::max();

    class EnumPropertyValue : public PropertyValue
    {
    public:
        EnumPropertyValue(void* _object, const EnumPropertyMetaInfo& _propertyMetaInfo);
        EnumPropertyValue(const void* _object, const EnumPropertyMetaInfo& _propertyMetaInfo);

        const EnumPropertyMetaInfo& getMetaInfo() const override;
        virtual size_t getValueIndex() const = 0;
        virtual bool setValueIndex(size_t _index) = 0;

        EGO_RTTI_VIRTUAL(EnumPropertyValue, PropertyValue);
    };

    template <typename Enum>
    requires std::is_enum_v<Enum>
    class TypedEnumPropertyValue final : public EnumPropertyValue
    {
    public:
        TypedEnumPropertyValue(void* _object, const TypedEnumPropertyMetaInfo<Enum>& _propertyMetaInfo);
        TypedEnumPropertyValue(const void* _object, const TypedEnumPropertyMetaInfo<Enum>& _propertyMetaInfo);

        const TypedEnumPropertyMetaInfo<Enum>& getMetaInfo() const override;
        size_t getValueIndex() const override;
        bool setValueIndex(size_t _index) override;

        EGO_RTTI_VIRTUAL(TypedEnumPropertyValue, EnumPropertyValue);
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Enum/EnumPropertyValue.hpp"
