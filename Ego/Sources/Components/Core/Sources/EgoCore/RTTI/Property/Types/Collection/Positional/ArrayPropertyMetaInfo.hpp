#pragma once

namespace ego::rtti
{
    template <typename Array>
    ArrayPropertyMetaInfo<Array>::ArrayPropertyMetaInfo(const char* _name, size_t _offset, const PropertyMetaInfo* _elementMetaInfo)
        : PositionalCollectionPropertyMetaInfo(_name, _offset, _elementMetaInfo)
    {
    }

    template <typename Array>
    size_t ArrayPropertyMetaInfo<Array>::getSize(const void* _value) const
    {
        return static_cast<const Array*>(_value)->size();
    }

    template <typename Array>
    void* ArrayPropertyMetaInfo<Array>::getElementAddress(void* _value, size_t _index) const
    {
        Array& array = *static_cast<Array*>(_value);

        return &array[_index];
    }

    template <typename Array>
    bool ArrayPropertyMetaInfo<Array>::canAddElement() const
    {
        return false;
    }

    template <typename Array>
    bool ArrayPropertyMetaInfo<Array>::canRemoveElement() const
    {
        return false;
    }

    template <typename Array>
    bool ArrayPropertyMetaInfo<Array>::addElement(void*) const
    {
        return false;
    }

    template <typename Array>
    bool ArrayPropertyMetaInfo<Array>::removeElement(void*, size_t) const
    {
        return false;
    }
} // namespace ego::rtti
