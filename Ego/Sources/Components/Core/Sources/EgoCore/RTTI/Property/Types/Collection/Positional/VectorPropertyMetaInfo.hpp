#pragma once

#include <concepts>

namespace ego::rtti
{
    template <typename Vector>
    VectorPropertyMetaInfo<Vector>::VectorPropertyMetaInfo(const char* _name, size_t _offset, const PropertyMetaInfo* _elementMetaInfo)
        : PositionalCollectionPropertyMetaInfo(_name, _offset, _elementMetaInfo)
    {
    }

    template <typename Vector>
    size_t VectorPropertyMetaInfo<Vector>::getSize(const void* _value) const
    {
        return static_cast<const Vector*>(_value)->size();
    }

    template <typename Vector>
    void* VectorPropertyMetaInfo<Vector>::getElementAddress(void* _value, size_t _index) const
    {
        Vector& vector = *static_cast<Vector*>(_value);

        return &vector[_index];
    }

    template <typename Vector>
    bool VectorPropertyMetaInfo<Vector>::canAddElement() const
    {
        return std::default_initializable<typename Vector::value_type>;
    }

    template <typename Vector>
    bool VectorPropertyMetaInfo<Vector>::canRemoveElement() const
    {
        return true;
    }

    template <typename Vector>
    bool VectorPropertyMetaInfo<Vector>::addElement(void* _value) const
    {
        if constexpr (std::default_initializable<typename Vector::value_type>)
        {
            static_cast<Vector*>(_value)->emplace_back();

            return true;
        }

        return false;
    }

    template <typename Vector>
    bool VectorPropertyMetaInfo<Vector>::removeElement(void* _value, size_t _index) const
    {
        Vector& vector = *static_cast<Vector*>(_value);
        if (_index >= vector.size())
        {
            return false;
        }

        vector.erase(vector.begin() + static_cast<typename Vector::difference_type>(_index));

        return true;
    }
} // namespace ego::rtti
