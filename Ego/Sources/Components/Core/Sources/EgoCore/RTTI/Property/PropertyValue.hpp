#pragma once

namespace ego::rtti
{
    template <typename Value>
    const Value& PropertyValue::getValue() const
    {
        return *static_cast<const Value*>(getValueAddress());
    }

    template <typename Value>
    Value* PropertyValue::tryGetMutableValue()
    {
        return static_cast<Value*>(tryGetMutableValueAddress());
    }
} // namespace ego::rtti
