#include <cstdint>
#include <type_traits>
#include <utility>

namespace ego::rtti
{
    template <typename MetaInfo, typename... Arguments>
    const PropertyMetaInfo& PropertyMetaInfoCollection::add(Arguments&&... _arguments)
    {
        static_assert(std::is_base_of_v<PropertyMetaInfo, MetaInfo>);

        std::unique_ptr<MetaInfo> metaInfo = std::make_unique<MetaInfo>(std::forward<Arguments>(_arguments)...);
        const PropertyMetaInfo& result = *metaInfo;

        m_metaInfos.emplace_back(std::move(metaInfo));

        return result;
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
