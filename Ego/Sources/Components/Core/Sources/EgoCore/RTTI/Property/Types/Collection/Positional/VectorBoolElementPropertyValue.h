#pragma once

#include "EgoCore/RTTI/Property/Types/Scalar/ScalarPropertyValue.h"

namespace ego::rtti
{
    class VectorBoolElementPropertyValue final : public ScalarPropertyValue
    {
    public:
        VectorBoolElementPropertyValue(bool* _value, const TypedScalarPropertyMetaInfo<bool>& _propertyMetaInfo);
        VectorBoolElementPropertyValue(const bool* _value, const TypedScalarPropertyMetaInfo<bool>& _propertyMetaInfo);

        const TypedScalarPropertyMetaInfo<bool>& getMetaInfo() const override;

        EGO_RTTI_VIRTUAL(VectorBoolElementPropertyValue, ScalarPropertyValue);

    private:
        bool m_value;
    };
} // namespace ego::rtti
