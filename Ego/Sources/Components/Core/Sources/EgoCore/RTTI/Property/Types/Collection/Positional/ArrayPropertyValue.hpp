#pragma once

#include <algorithm>
#include <iterator>
#include <type_traits>

namespace ego::rtti
{
    template <typename Array>
    ArrayPropertyValue<Array>::ArrayPropertyValue(void* _object, const ArrayPropertyMetaInfo<Array>& _propertyMetaInfo)
        : PositionalCollectionPropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename Array>
    ArrayPropertyValue<Array>::ArrayPropertyValue(const void* _object, const ArrayPropertyMetaInfo<Array>& _propertyMetaInfo)
        : PositionalCollectionPropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename Array>
    const ArrayPropertyMetaInfo<Array>& ArrayPropertyValue<Array>::getMetaInfo() const
    {
        return static_cast<const ArrayPropertyMetaInfo<Array>&>(PropertyValue::getMetaInfo());
    }

    template <typename Array>
    size_t ArrayPropertyValue<Array>::getSize() const
    {
        return PropertyValue::getValue<Array>().size();
    }

    template <typename Array>
    PropertyValuePointer ArrayPropertyValue<Array>::getElement(size_t _index)
    {
        Array* mutableArray = PropertyValue::tryGetMutableValue<Array>();
        const Array& array = mutableArray ? *mutableArray : PropertyValue::getValue<Array>();
        if (_index >= array.size())
        {
            return nullptr;
        }

        if (mutableArray)
        {
            return getMetaInfo().getElementMetaInfo().makePropertyValue(&(*mutableArray)[_index]);
        }

        return getMetaInfo().getElementMetaInfo().makePropertyValue(&array[_index]);
    }

    template <typename Array>
    bool ArrayPropertyValue<Array>::setElement(size_t _index, const PropertyValue& _element)
    {
        Array* array = PropertyValue::tryGetMutableValue<Array>();
        if (!array || _index >= array->size() || &_element.getMetaInfo() != &getMetaInfo().getElementMetaInfo())
        {
            return false;
        }

        using Element = typename Array::value_type;
        const Element* const source = static_cast<const Element*>(_element.getValueAddress());
        if (&(*array)[_index] == source)
        {
            return true;
        }

        if constexpr (std::is_copy_assignable_v<Element>)
        {
            (*array)[_index] = *source;

            return true;
        }

        return false;
    }

    template <typename Array>
    bool ArrayPropertyValue<Array>::canResize() const
    {
        return false;
    }

    template <typename Array>
    bool ArrayPropertyValue<Array>::resize(size_t)
    {
        return false;
    }

    template <typename Array>
    bool ArrayPropertyValue<Array>::canAddElement() const
    {
        return false;
    }

    template <typename Array>
    bool ArrayPropertyValue<Array>::addElement()
    {
        return false;
    }

    template <typename Array>
    bool ArrayPropertyValue<Array>::canRemoveElement() const
    {
        return false;
    }

    template <typename Array>
    bool ArrayPropertyValue<Array>::removeElement(size_t)
    {
        return false;
    }

    template <typename Array>
    bool ArrayPropertyValue<Array>::canMoveElement() const
    {
        return !isReadOnly() && std::permutable<typename Array::iterator>;
    }

    template <typename Array>
    bool ArrayPropertyValue<Array>::moveElement(size_t _sourceIndex, size_t _destinationIndex)
    {
        Array* array = PropertyValue::tryGetMutableValue<Array>();
        if (!array || _sourceIndex >= array->size() || _destinationIndex >= array->size())
        {
            return false;
        }

        if (_sourceIndex == _destinationIndex)
        {
            return true;
        }

        if constexpr (std::permutable<typename Array::iterator>)
        {
            const typename Array::iterator begin = array->begin();
            if (_sourceIndex < _destinationIndex)
            {
                std::rotate(begin + _sourceIndex, begin + _sourceIndex + 1, begin + _destinationIndex + 1);
            }
            else
            {
                std::rotate(begin + _destinationIndex, begin + _sourceIndex, begin + _sourceIndex + 1);
            }

            return true;
        }
        else
        {
            return false;
        }
    }
} // namespace ego::rtti
