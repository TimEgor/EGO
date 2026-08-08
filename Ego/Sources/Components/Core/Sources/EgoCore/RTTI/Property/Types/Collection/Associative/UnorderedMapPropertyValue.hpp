#pragma once

#include <concepts>

namespace ego::rtti
{
    template <typename UnorderedMap>
    UnorderedMapPropertyValue<UnorderedMap>::UnorderedMapPropertyValue(void* _object, const UnorderedMapPropertyMetaInfo<UnorderedMap>& _propertyMetaInfo)
        : AssociativeCollectionPropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename UnorderedMap>
    UnorderedMapPropertyValue<UnorderedMap>::UnorderedMapPropertyValue(const void* _object, const UnorderedMapPropertyMetaInfo<UnorderedMap>& _propertyMetaInfo)
        : AssociativeCollectionPropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename UnorderedMap>
    const UnorderedMapPropertyMetaInfo<UnorderedMap>& UnorderedMapPropertyValue<UnorderedMap>::getMetaInfo() const
    {
        return static_cast<const UnorderedMapPropertyMetaInfo<UnorderedMap>&>(PropertyValue::getMetaInfo());
    }

    template <typename UnorderedMap>
    size_t UnorderedMapPropertyValue<UnorderedMap>::getSize() const
    {
        return PropertyValue::getValue<UnorderedMap>().size();
    }

    template <typename UnorderedMap>
    void UnorderedMapPropertyValue<UnorderedMap>::visitElements(const ElementVisitor& _visitor)
    {
        if (!_visitor)
        {
            return;
        }

        UnorderedMap* mutableUnorderedMap = PropertyValue::tryGetMutableValue<UnorderedMap>();
        if (!mutableUnorderedMap)
        {
            const UnorderedMap& unorderedMap = PropertyValue::getValue<UnorderedMap>();
            for (const typename UnorderedMap::value_type& element : unorderedMap)
            {
                PropertyValuePointer key = getMetaInfo().getKeyMetaInfo().makePropertyValue(&element.first);
                PropertyValuePointer mappedValue = getMetaInfo().getMappedMetaInfo().makePropertyValue(&element.second);
                _visitor(*key, *mappedValue);
            }

            return;
        }

        typename UnorderedMap::iterator iterator = mutableUnorderedMap->begin();
        while (iterator != mutableUnorderedMap->end())
        {
            PropertyValuePointer key = getMetaInfo().getKeyMetaInfo().makePropertyValue(&iterator->first);
            PropertyValuePointer mappedValue = getMetaInfo().getMappedMetaInfo().makePropertyValue(&iterator->second);

            if (_visitor(*key, *mappedValue) == AssociativeCollectionElementAction::Remove)
            {
                iterator = mutableUnorderedMap->erase(iterator);
            }
            else
            {
                ++iterator;
            }
        }
    }

    template <typename UnorderedMap>
    bool UnorderedMapPropertyValue<UnorderedMap>::canAddElement() const
    {
        if (isReadOnly())
        {
            return false;
        }

        if constexpr (std::default_initializable<typename UnorderedMap::key_type> && std::default_initializable<typename UnorderedMap::mapped_type>)
        {
            const UnorderedMap& unorderedMap = PropertyValue::getValue<UnorderedMap>();

            return unorderedMap.find(typename UnorderedMap::key_type()) == unorderedMap.end();
        }

        return false;
    }

    template <typename UnorderedMap>
    bool UnorderedMapPropertyValue<UnorderedMap>::addElement()
    {
        if constexpr (std::default_initializable<typename UnorderedMap::key_type> && std::default_initializable<typename UnorderedMap::mapped_type>)
        {
            UnorderedMap* unorderedMap = PropertyValue::tryGetMutableValue<UnorderedMap>();
            if (!unorderedMap)
            {
                return false;
            }

            return unorderedMap->try_emplace(typename UnorderedMap::key_type()).second;
        }

        return false;
    }
} // namespace ego::rtti
