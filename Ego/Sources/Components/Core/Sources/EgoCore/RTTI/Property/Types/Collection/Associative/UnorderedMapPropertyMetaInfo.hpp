#pragma once

#include <concepts>

namespace ego::rtti
{
    template <typename UnorderedMap>
    UnorderedMapPropertyMetaInfo<UnorderedMap>::UnorderedMapPropertyMetaInfo(
        const char* _name,
        size_t _offset,
        const PropertyMetaInfo* _keyMetaInfo,
        const PropertyMetaInfo* _mappedMetaInfo)
        : AssociativeCollectionPropertyMetaInfo(_name, _offset, _keyMetaInfo, _mappedMetaInfo)
    {
    }

    template <typename UnorderedMap>
    size_t UnorderedMapPropertyMetaInfo<UnorderedMap>::getSize(const void* _value) const
    {
        return static_cast<const UnorderedMap*>(_value)->size();
    }

    template <typename UnorderedMap>
    void UnorderedMapPropertyMetaInfo<UnorderedMap>::visitElements(void* _value, const ElementVisitor& _visitor) const
    {
        UnorderedMap& unorderedMap = *static_cast<UnorderedMap*>(_value);
        typename UnorderedMap::iterator iterator = unorderedMap.begin();
        while (iterator != unorderedMap.end())
        {
            if (_visitor(&iterator->first, &iterator->second))
            {
                iterator = unorderedMap.erase(iterator);
            }
            else
            {
                ++iterator;
            }
        }
    }

    template <typename UnorderedMap>
    bool UnorderedMapPropertyMetaInfo<UnorderedMap>::canAddElement() const
    {
        return std::default_initializable<typename UnorderedMap::key_type> && std::default_initializable<typename UnorderedMap::mapped_type>;
    }

    template <typename UnorderedMap>
    bool UnorderedMapPropertyMetaInfo<UnorderedMap>::addElement(void* _value) const
    {
        if constexpr (std::default_initializable<typename UnorderedMap::key_type> && std::default_initializable<typename UnorderedMap::mapped_type>)
        {
            UnorderedMap& unorderedMap = *static_cast<UnorderedMap*>(_value);

            return unorderedMap.try_emplace(typename UnorderedMap::key_type()).second;
        }

        return false;
    }
} // namespace ego::rtti
