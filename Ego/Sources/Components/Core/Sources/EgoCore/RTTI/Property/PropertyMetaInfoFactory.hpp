#pragma once

#include <array>
#include <concepts>
#include <map>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "EgoCore/FileName/FileName.h"

#include "EgoCore/RTTI/Property/Types/Collection/Associative/MapPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Collection/Associative/UnorderedMapPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Collection/Positional/ArrayPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Collection/Positional/VectorPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Enum/EnumPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Scalar/ScalarPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/String/FileNamePropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/String/StdStringPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Struct/StructPropertyMetaInfo.h"

namespace ego::rtti
{
    namespace detail
    {
        template <typename T>
        struct IsArray final : std::false_type
        {
        };

        template <typename T, size_t Size>
        struct IsArray<std::array<T, Size>> final : std::true_type
        {
        };

        template <typename T>
        struct IsVector final : std::false_type
        {
        };

        template <typename T, typename Allocator>
        struct IsVector<std::vector<T, Allocator>> final : std::true_type
        {
        };

        template <typename T>
        struct IsMap final : std::false_type
        {
        };

        template <typename Key, typename Mapped, typename Compare, typename Allocator>
        struct IsMap<std::map<Key, Mapped, Compare, Allocator>> final : std::true_type
        {
        };

        template <typename T>
        struct IsUnorderedMap final : std::false_type
        {
        };

        template <typename Key, typename Mapped, typename Hash, typename KeyEqual, typename Allocator>
        struct IsUnorderedMap<std::unordered_map<Key, Mapped, Hash, KeyEqual, Allocator>> final : std::true_type
        {
        };

        template <typename T>
        concept ReflectedType = requires {
            { T::GetMetaInfo() } -> std::same_as<const TypeMetaInfo&>;
        };
    } // namespace detail

    template <typename Value, typename... PropertyArguments>
    const PropertyMetaInfo& AddPropertyMetaInfo(
        PropertyMetaInfoCollection& _propertyMetaInfos,
        const char* _name,
        size_t _offset,
        PropertyArguments&&... _propertyArguments)
    {
        using DeclaredType = std::remove_reference_t<Value>;
        using Type = std::remove_cv_t<DeclaredType>;
        constexpr bool IsConst = std::is_const_v<DeclaredType>;

        if constexpr (std::is_arithmetic_v<Type>)
        {
            return _propertyMetaInfos.add<TypedScalarPropertyMetaInfo<Type>>(_name, _offset, IsConst, std::forward<PropertyArguments>(_propertyArguments)...);
        }
        else if constexpr (std::is_same_v<Type, std::string>)
        {
            return _propertyMetaInfos.add<StdStringPropertyMetaInfo>(_name, _offset, IsConst, std::forward<PropertyArguments>(_propertyArguments)...);
        }
        else if constexpr (std::is_same_v<Type, FileName>)
        {
            return _propertyMetaInfos.add<FileNamePropertyMetaInfo>(_name, _offset, IsConst, std::forward<PropertyArguments>(_propertyArguments)...);
        }
        else if constexpr (std::is_enum_v<Type>)
        {
            return _propertyMetaInfos.add<TypedEnumPropertyMetaInfo<Type>>(_name, _offset, IsConst, std::forward<PropertyArguments>(_propertyArguments)...);
        }
        else if constexpr (detail::IsArray<Type>::value)
        {
            using Element = typename Type::value_type;
            static const PropertyMetaInfoCollection ElementMetaInfos = []()
            {
                PropertyMetaInfoCollection result;
                AddPropertyMetaInfo<Element>(result, "Element", 0);

                return result;
            }();

            return _propertyMetaInfos.add<ArrayPropertyMetaInfo<Type>>(
                _name,
                _offset,
                IsConst,
                ElementMetaInfos.getMetaInfo(0),
                std::forward<PropertyArguments>(_propertyArguments)...);
        }
        else if constexpr (detail::IsVector<Type>::value)
        {
            using Element = typename Type::value_type;
            static const PropertyMetaInfoCollection ElementMetaInfos = []()
            {
                PropertyMetaInfoCollection result;
                AddPropertyMetaInfo<Element>(result, "Element", 0);

                return result;
            }();

            return _propertyMetaInfos.add<VectorPropertyMetaInfo<Type>>(
                _name,
                _offset,
                IsConst,
                ElementMetaInfos.getMetaInfo(0),
                std::forward<PropertyArguments>(_propertyArguments)...);
        }
        else if constexpr (detail::IsMap<Type>::value)
        {
            using Key = typename Type::key_type;
            using Mapped = typename Type::mapped_type;
            static const PropertyMetaInfoCollection KeyMetaInfos = []()
            {
                PropertyMetaInfoCollection result;
                AddPropertyMetaInfo<Key>(result, "Key", 0);

                return result;
            }();
            static const PropertyMetaInfoCollection MappedMetaInfos = []()
            {
                PropertyMetaInfoCollection result;
                AddPropertyMetaInfo<Mapped>(result, "Value", 0);

                return result;
            }();

            return _propertyMetaInfos.add<MapPropertyMetaInfo<Type>>(
                _name,
                _offset,
                IsConst,
                KeyMetaInfos.getMetaInfo(0),
                MappedMetaInfos.getMetaInfo(0),
                std::forward<PropertyArguments>(_propertyArguments)...);
        }
        else if constexpr (detail::IsUnorderedMap<Type>::value)
        {
            using Key = typename Type::key_type;
            using Mapped = typename Type::mapped_type;
            static const PropertyMetaInfoCollection KeyMetaInfos = []()
            {
                PropertyMetaInfoCollection result;
                AddPropertyMetaInfo<Key>(result, "Key", 0);

                return result;
            }();
            static const PropertyMetaInfoCollection MappedMetaInfos = []()
            {
                PropertyMetaInfoCollection result;
                AddPropertyMetaInfo<Mapped>(result, "Value", 0);

                return result;
            }();

            return _propertyMetaInfos.add<UnorderedMapPropertyMetaInfo<Type>>(
                _name,
                _offset,
                IsConst,
                KeyMetaInfos.getMetaInfo(0),
                MappedMetaInfos.getMetaInfo(0),
                std::forward<PropertyArguments>(_propertyArguments)...);
        }
        else if constexpr (detail::ReflectedType<Type>)
        {
            return _propertyMetaInfos.add<TypedStructPropertyMetaInfo<Type>>(
                _name,
                _offset,
                IsConst,
                GetTypeMetaInfo<Type>(),
                std::forward<PropertyArguments>(_propertyArguments)...);
        }
        else
        {
            static_assert(false, "The property type is not supported by Ego RTTI.");
        }
    }
} // namespace ego::rtti
