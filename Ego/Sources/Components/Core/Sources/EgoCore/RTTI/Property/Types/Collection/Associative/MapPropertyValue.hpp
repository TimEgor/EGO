#pragma once

#include <concepts>

namespace ego::rtti
{
    template <typename Map>
    MapPropertyValue<Map>::MapPropertyValue(void* _object, const MapPropertyMetaInfo<Map>& _propertyMetaInfo)
        : AssociativeCollectionPropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename Map>
    MapPropertyValue<Map>::MapPropertyValue(const void* _object, const MapPropertyMetaInfo<Map>& _propertyMetaInfo)
        : AssociativeCollectionPropertyValue(_object, _propertyMetaInfo)
    {
    }

    template <typename Map>
    const MapPropertyMetaInfo<Map>& MapPropertyValue<Map>::getMetaInfo() const
    {
        return static_cast<const MapPropertyMetaInfo<Map>&>(PropertyValue::getMetaInfo());
    }

    template <typename Map>
    size_t MapPropertyValue<Map>::getSize() const
    {
        return PropertyValue::getValue<Map>().size();
    }

    template <typename Map>
    void MapPropertyValue<Map>::visitElements(const ElementVisitor& _visitor)
    {
        if (!_visitor)
        {
            return;
        }

        Map* mutableMap = PropertyValue::tryGetMutableValue<Map>();
        if (!mutableMap)
        {
            const Map& map = PropertyValue::getValue<Map>();
            for (const typename Map::value_type& element : map)
            {
                PropertyValuePointer key = getMetaInfo().getKeyMetaInfo().makePropertyValue(&element.first);
                PropertyValuePointer mappedValue = getMetaInfo().getMappedMetaInfo().makePropertyValue(&element.second);
                _visitor(*key, *mappedValue);
            }

            return;
        }

        typename Map::iterator iterator = mutableMap->begin();
        while (iterator != mutableMap->end())
        {
            PropertyValuePointer key = getMetaInfo().getKeyMetaInfo().makePropertyValue(&iterator->first);
            PropertyValuePointer mappedValue = getMetaInfo().getMappedMetaInfo().makePropertyValue(&iterator->second);

            if (_visitor(*key, *mappedValue) == AssociativeCollectionElementAction::Remove)
            {
                iterator = mutableMap->erase(iterator);
            }
            else
            {
                ++iterator;
            }
        }
    }

    template <typename Map>
    bool MapPropertyValue<Map>::canAddElement() const
    {
        if (isReadOnly())
        {
            return false;
        }

        if constexpr (std::default_initializable<typename Map::key_type> && std::default_initializable<typename Map::mapped_type>)
        {
            const Map& map = PropertyValue::getValue<Map>();

            return map.find(typename Map::key_type()) == map.end();
        }

        return false;
    }

    template <typename Map>
    bool MapPropertyValue<Map>::addElement()
    {
        if constexpr (std::default_initializable<typename Map::key_type> && std::default_initializable<typename Map::mapped_type>)
        {
            Map* map = PropertyValue::tryGetMutableValue<Map>();
            if (!map)
            {
                return false;
            }

            return map->try_emplace(typename Map::key_type()).second;
        }

        return false;
    }
} // namespace ego::rtti
