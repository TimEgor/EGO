#pragma once

#include "EgoCore/RTTI/Property/PropertyMetaInfo.h"

namespace ego::rtti
{
    class TypeMetaInfo;

    template <typename Struct>
    class StructPropertyValue;

    class StructPropertyMetaInfo : public PropertyMetaInfo
    {
    public:
        StructPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, const TypeMetaInfo& _valueTypeMetaInfo);

        const TypeMetaInfo& getValueTypeMetaInfo() const;

        EGO_RTTI_VIRTUAL(StructPropertyMetaInfo, PropertyMetaInfo);

    private:
        const TypeMetaInfo& m_valueTypeMetaInfo;
    };

    template <typename Struct>
    class TypedStructPropertyMetaInfo final : public StructPropertyMetaInfo
    {
    public:
        TypedStructPropertyMetaInfo(const char* _name, size_t _offset, bool _isConst, const TypeMetaInfo& _valueTypeMetaInfo);

        EGO_RTTI_VIRTUAL(TypedStructPropertyMetaInfo, StructPropertyMetaInfo);

        PropertyValuePointer makePropertyValue(void* _object) const override;
        PropertyValuePointer makePropertyValue(const void* _object) const override;
    };
} // namespace ego::rtti

#include "EgoCore/RTTI/Property/Types/Struct/StructPropertyMetaInfo.hpp"
