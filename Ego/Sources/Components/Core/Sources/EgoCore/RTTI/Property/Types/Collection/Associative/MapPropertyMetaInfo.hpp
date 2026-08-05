#pragma once

#include <concepts>

namespace ego::rtti
{
    template <typename Map>
    MapPropertyMetaInfo<Map>::MapPropertyMetaInfo(
        const char* _name,
        size_t _offset,
        const PropertyMetaInfo* _keyMetaInfo,
        const PropertyMetaInfo* _mappedMetaInfo)
        : AssociativeCollectionPropertyMetaInfo(_name, _offset, _keyMetaInfo, _mappedMetaInfo)
    {
    }

    template <typename Map>
    size_t MapPropertyMetaInfo<Map>::getSize(const void* _value) const
    {
        return static_cast<const Map*>(_value)->size();
    }

    template <typename Map>
    void MapPropertyMetaInfo<Map>::visitElements(void* _value, const ElementVisitor& _visitor) const
    {
        Map& map = *static_cast<Map*>(_value);
        typename Map::iterator iterator = map.begin();
        while (iterator != map.end())
        {
            if (_visitor(&iterator->first, &iterator->second))
            {
                iterator = map.erase(iterator);
            }
            else
            {
                ++iterator;
            }
        }
    }

    template <typename Map>
    bool MapPropertyMetaInfo<Map>::canAddElement() const
    {
        return std::default_initializable<typename Map::key_type> && std::default_initializable<typename Map::mapped_type>;
    }

    template <typename Map>
    bool MapPropertyMetaInfo<Map>::addElement(void* _value) const
    {
        if constexpr (std::default_initializable<typename Map::key_type> && std::default_initializable<typename Map::mapped_type>)
        {
            Map& map = *static_cast<Map*>(_value);

            return map.try_emplace(typename Map::key_type()).second;
        }

        return false;
    }
} // namespace ego::rtti
