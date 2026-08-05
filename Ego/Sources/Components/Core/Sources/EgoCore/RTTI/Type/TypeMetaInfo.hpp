#include <cstdint>
#include <type_traits>
#include <utility>

namespace ego::rtti
{
    template <typename Value>
    Value& TypeMetaInfo::PropertyIterator::getValue(void* _object) const
    {
        return *static_cast<Value*>(getValueAddress(_object));
    }

    template <typename Value>
    const Value& TypeMetaInfo::PropertyIterator::getValue(const void* _object) const
    {
        return *static_cast<const Value*>(getValueAddress(_object));
    }

    template <typename T>
    const char* GetTypeMetaInfoName()
    {
        return T::GetMetaInfoTypeName();
    }

    template <typename T>
    const TypeMetaInfo& GetTypeMetaInfo()
    {
        return T::GetMetaInfo();
    }

    template <typename Type, typename Base>
    size_t GetBaseTypeOffset()
    {
        static_assert(std::is_base_of_v<Base, Type>);

        constexpr uintptr_t ObjectAddress = 0x10000;
        Type* object = reinterpret_cast<Type*>(ObjectAddress);
        Base* base = static_cast<Base*>(object);

        return reinterpret_cast<uintptr_t>(base) - ObjectAddress;
    }

    template <typename Type, typename... Bases>
    TypeMetaInfo RegistryTypeMetaInfo()
    {
        PropertyMetaInfoCollection propertyMetaInfos;
        if constexpr (requires { Type::template RegistryPropertyMetaInfoCollection<Type>(); })
        {
            propertyMetaInfos = Type::template RegistryPropertyMetaInfoCollection<Type>();
        }

        return TypeMetaInfo(GetTypeMetaInfoID<Type>(), {{&(GetTypeMetaInfo<Bases>()), GetBaseTypeOffset<Type, Bases>()}...}, std::move(propertyMetaInfos));
    }

    template <typename Type, typename Base>
    bool IsBasedOn()
    {
        return GetTypeMetaInfo<Type>().isBasedOn(GetTypeMetaInfo<Base>());
    }

    template <typename Type>
    bool IsObjectBasedOn(TypeMetaInfoID _baseTypeID)
    {
        return GetTypeMetaInfo<Type>().isBasedOn(_baseTypeID);
    }

    template <typename Base, typename Type>
    bool IsObjectBasedOn(const Type& _object)
    {
        return _object.getObjectTypeMetaInfo().isBasedOn(GetTypeMetaInfo<Base>());
    }

    template <typename Type>
    bool IsObjectBasedOn(const Type& _object, TypeMetaInfoID _baseTypeID)
    {
        return _object.getObjectTypeMetaInfo().isBasedOn(_baseTypeID);
    }

    template <typename Type, typename Base>
    Type* CastTo(const Base* _object)
    {
        return GetTypeMetaInfo<Type>().castTo(_object, GetTypeMetaInfo<Base>());
    }
} // namespace ego::rtti
