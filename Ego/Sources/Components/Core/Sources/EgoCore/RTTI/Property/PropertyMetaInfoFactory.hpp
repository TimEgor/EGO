#include <array>
#include <concepts>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Pointer/IntrusivePointer.h"
#include "EgoCore/Pointer/Pointer.h"

#include "EgoCore/RTTI/Property/Types/Collection/Associative/MapPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Collection/Associative/UnorderedMapPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Collection/Positional/ArrayPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Collection/Positional/VectorPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Enum/EnumPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Object/ObjectPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Scalar/ScalarPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/String/FileNamePropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/String/StdStringPropertyMetaInfo.h"
#include "EgoCore/RTTI/Property/Types/Struct/StructPropertyMetaInfo.h"
#include "EgoCore/RTTI/Type/TypeMetaInfo.h"

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
        struct IsObjectPointer final : std::false_type
        {
        };

        template <typename T>
        struct IsObjectPointer<SharedPointer<T>> final : std::true_type
        {
        };

        template <typename T>
        struct IsObjectPointer<IntrusivePointer<T>> final : std::true_type
        {
        };

        template <typename T>
        concept ReflectedType = requires {
            { T::GetMetaInfo() } -> std::same_as<const TypeMetaInfo&>;
        };
    } // namespace detail

    template <typename Value>
    PropertyMetaInfoPointer MakePropertyMetaInfo(const char* _name, size_t _offset)
    {
        using Type = std::remove_cvref_t<Value>;

        if constexpr (std::is_arithmetic_v<Type>)
        {
            return std::make_unique<ScalarPropertyMetaInfo<Type>>(_name, _offset);
        }
        else if constexpr (std::is_same_v<Type, std::string>)
        {
            return std::make_unique<StdStringPropertyMetaInfo>(_name, _offset);
        }
        else if constexpr (std::is_same_v<Type, FileName>)
        {
            return std::make_unique<FileNamePropertyMetaInfo>(_name, _offset);
        }
        else if constexpr (std::is_enum_v<Type>)
        {
            return std::make_unique<EnumPropertyMetaInfo<Type>>(_name, _offset);
        }
        else if constexpr (detail::IsArray<Type>::value)
        {
            using Element = typename Type::value_type;
            static const PropertyMetaInfoPointer ElementMetaInfo = MakePropertyMetaInfo<Element>("Element", 0);

            return std::make_unique<ArrayPropertyMetaInfo<Type>>(_name, _offset, ElementMetaInfo.get());
        }
        else if constexpr (detail::IsVector<Type>::value)
        {
            using Element = typename Type::value_type;
            static_assert(!std::is_same_v<Element, bool>, "std::vector<bool> properties are not supported.");

            static const PropertyMetaInfoPointer ElementMetaInfo = MakePropertyMetaInfo<Element>("Element", 0);

            return std::make_unique<VectorPropertyMetaInfo<Type>>(_name, _offset, ElementMetaInfo.get());
        }
        else if constexpr (detail::IsMap<Type>::value)
        {
            using Key = typename Type::key_type;
            using Mapped = typename Type::mapped_type;
            static const PropertyMetaInfoPointer KeyMetaInfo = MakePropertyMetaInfo<Key>("Key", 0);
            static const PropertyMetaInfoPointer MappedMetaInfo = MakePropertyMetaInfo<Mapped>("Value", 0);

            return std::make_unique<MapPropertyMetaInfo<Type>>(_name, _offset, KeyMetaInfo.get(), MappedMetaInfo.get());
        }
        else if constexpr (detail::IsUnorderedMap<Type>::value)
        {
            using Key = typename Type::key_type;
            using Mapped = typename Type::mapped_type;
            static const PropertyMetaInfoPointer KeyMetaInfo = MakePropertyMetaInfo<Key>("Key", 0);
            static const PropertyMetaInfoPointer MappedMetaInfo = MakePropertyMetaInfo<Mapped>("Value", 0);

            return std::make_unique<UnorderedMapPropertyMetaInfo<Type>>(_name, _offset, KeyMetaInfo.get(), MappedMetaInfo.get());
        }
        else if constexpr (detail::IsObjectPointer<Type>::value)
        {
            return std::make_unique<ObjectPropertyMetaInfo<Type>>(_name, _offset);
        }
        else if constexpr (detail::ReflectedType<Type>)
        {
            return std::make_unique<StructPropertyMetaInfo<Type>>(_name, _offset, &GetTypeMetaInfo<Type>());
        }
        else
        {
            static_assert(false, "The property type is not supported by Ego RTTI.");
        }
    }

    template <typename... Properties>
    PropertyMetaInfoCollection MakePropertyMetaInfoCollection(Properties&&... _properties)
    {
        static_assert((std::is_same_v<std::remove_cvref_t<Properties>, PropertyMetaInfoPointer> && ...));

        PropertyMetaInfoCollection result;
        result.reserve(sizeof...(Properties));
        (result.emplace_back(std::forward<Properties>(_properties)), ...);

        return result;
    }
} // namespace ego::rtti
