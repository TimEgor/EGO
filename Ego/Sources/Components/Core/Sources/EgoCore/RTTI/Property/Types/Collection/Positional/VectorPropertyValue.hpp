#pragma once

#include <algorithm>
#include <concepts>
#include <iterator>
#include <type_traits>

namespace ego::rtti
{
    template <typename Vector>
    VectorPropertyValue<Vector>::VectorPropertyValue(void* _object, const VectorPropertyMetaInfo<Vector>& _propertyMetaInfo)
        : PositionalCollectionPropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename Vector>
    VectorPropertyValue<Vector>::VectorPropertyValue(const void* _object, const VectorPropertyMetaInfo<Vector>& _propertyMetaInfo)
        : PositionalCollectionPropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename Vector>
    const VectorPropertyMetaInfo<Vector>& VectorPropertyValue<Vector>::getMetaInfo() const
    {
        return static_cast<const VectorPropertyMetaInfo<Vector>&>(PropertyValue::getMetaInfo());
    }

    template <typename Vector>
    size_t VectorPropertyValue<Vector>::getSize() const
    {
        return PropertyValue::getValue<Vector>().size();
    }

    template <typename Vector>
    PropertyValuePointer VectorPropertyValue<Vector>::getElement(size_t _index)
    {
        Vector* mutableVector = PropertyValue::tryGetMutableValue<Vector>();
        const Vector& vector = mutableVector ? *mutableVector : PropertyValue::getValue<Vector>();
        if (_index >= vector.size())
        {
            return nullptr;
        }

        using Element = typename Vector::value_type;
        if constexpr (std::is_same_v<Element, bool>)
        {
            const TypedScalarPropertyMetaInfo<bool>& elementMetaInfo =
                static_cast<const TypedScalarPropertyMetaInfo<bool>&>(getMetaInfo().getElementMetaInfo());

            bool value = vector[_index];
            if (mutableVector)
            {
                return MakeIntrusive<VectorBoolElementPropertyValue>(&value, elementMetaInfo);
            }

            return MakeIntrusive<VectorBoolElementPropertyValue>(static_cast<const bool*>(&value), elementMetaInfo);
        }
        else if (mutableVector)
        {
            return getMetaInfo().getElementMetaInfo().makePropertyValue(&(*mutableVector)[_index]);
        }
        else
        {
            return getMetaInfo().getElementMetaInfo().makePropertyValue(&vector[_index]);
        }
    }

    template <typename Vector>
    bool VectorPropertyValue<Vector>::setElement(size_t _index, const PropertyValue& _element)
    {
        Vector* vector = PropertyValue::tryGetMutableValue<Vector>();
        if (!vector || _index >= vector->size() || &_element.getMetaInfo() != &getMetaInfo().getElementMetaInfo())
        {
            return false;
        }

        using Element = typename Vector::value_type;
        const Element* const source = static_cast<const Element*>(_element.getValueAddress());
        if constexpr (std::is_copy_assignable_v<Element>)
        {
            if constexpr (!std::is_same_v<Element, bool>)
            {
                if (&(*vector)[_index] == source)
                {
                    return true;
                }
            }

            (*vector)[_index] = *source;

            return true;
        }

        return false;
    }

    template <typename Vector>
    bool VectorPropertyValue<Vector>::canResize() const
    {
        return !isReadOnly() && std::default_initializable<typename Vector::value_type>;
    }

    template <typename Vector>
    bool VectorPropertyValue<Vector>::resize(size_t _size)
    {
        Vector* vector = PropertyValue::tryGetMutableValue<Vector>();
        if (!vector)
        {
            return false;
        }

        if (_size < vector->size())
        {
            vector->erase(vector->begin() + static_cast<typename Vector::difference_type>(_size), vector->end());

            return true;
        }

        if (_size == vector->size())
        {
            return true;
        }

        if constexpr (std::default_initializable<typename Vector::value_type>)
        {
            vector->resize(_size);

            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename Vector>
    bool VectorPropertyValue<Vector>::canAddElement() const
    {
        return !isReadOnly() && std::default_initializable<typename Vector::value_type>;
    }

    template <typename Vector>
    bool VectorPropertyValue<Vector>::addElement()
    {
        Vector* vector = PropertyValue::tryGetMutableValue<Vector>();
        if (!vector)
        {
            return false;
        }

        if constexpr (std::default_initializable<typename Vector::value_type>)
        {
            vector->emplace_back();

            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename Vector>
    bool VectorPropertyValue<Vector>::canRemoveElement() const
    {
        return !isReadOnly();
    }

    template <typename Vector>
    bool VectorPropertyValue<Vector>::removeElement(size_t _index)
    {
        Vector* vector = PropertyValue::tryGetMutableValue<Vector>();
        if (!vector || _index >= vector->size())
        {
            return false;
        }

        vector->erase(vector->begin() + static_cast<typename Vector::difference_type>(_index));

        return true;
    }

    template <typename Vector>
    bool VectorPropertyValue<Vector>::canMoveElement() const
    {
        using Element = typename Vector::value_type;

        return !isReadOnly() && (std::is_same_v<Element, bool> || std::permutable<typename Vector::iterator>);
    }

    template <typename Vector>
    bool VectorPropertyValue<Vector>::moveElement(size_t _sourceIndex, size_t _destinationIndex)
    {
        Vector* vector = PropertyValue::tryGetMutableValue<Vector>();
        if (!vector || _sourceIndex >= vector->size() || _destinationIndex >= vector->size())
        {
            return false;
        }

        if (_sourceIndex == _destinationIndex)
        {
            return true;
        }

        using Element = typename Vector::value_type;
        if constexpr (std::is_same_v<Element, bool>)
        {
            const bool source = (*vector)[_sourceIndex];
            if (_sourceIndex < _destinationIndex)
            {
                for (size_t index = _sourceIndex; index < _destinationIndex; ++index)
                {
                    (*vector)[index] = (*vector)[index + 1];
                }
            }
            else
            {
                for (size_t index = _sourceIndex; index > _destinationIndex; --index)
                {
                    (*vector)[index] = (*vector)[index - 1];
                }
            }

            (*vector)[_destinationIndex] = source;

            return true;
        }
        else if constexpr (std::permutable<typename Vector::iterator>)
        {
            const typename Vector::iterator begin = vector->begin();
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
