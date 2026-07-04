#pragma once

#include <vector>

#include "EgoCore/Hash/CRC32.h"

namespace ego::rtti
{
    using TypeMetaInfoID = uint32_t;
    inline constexpr TypeMetaInfoID InvalidTypeMetaInfoID = 0;

    class TypeMetaInfo final
    {
    public:
        struct ParentTypeMetaInfoContext final
        {
            const TypeMetaInfo* m_info = nullptr;
            size_t m_typeOffset = 0;

            ParentTypeMetaInfoContext() = default;

            ParentTypeMetaInfoContext(const TypeMetaInfo* _info, size_t _offset)
                : m_info(_info),
                  m_typeOffset(_offset)
            {
            }
        };

        using ParentTypeMetaInfoCollection = std::vector<ParentTypeMetaInfoContext>;

        TypeMetaInfo(TypeMetaInfoID _id, size_t _size, ParentTypeMetaInfoCollection&& _parentTypeMetaInfos = ParentTypeMetaInfoCollection());
        TypeMetaInfo(const TypeMetaInfo&) = default;
        TypeMetaInfo(TypeMetaInfo&&) = default;

        bool isBasedOn(const TypeMetaInfo& _baseType) const;
        bool isBasedOn(TypeMetaInfoID _baseTypeID) const;
        void* castTo(void* _object, const TypeMetaInfo& _baseType) const;

    private:
        ParentTypeMetaInfoCollection m_parentTypeMetaInfos;
        const size_t m_size = 0;
        const TypeMetaInfoID m_id = 0;
    };

    template <typename T>
    const char* GetTypeMetaInfoName()
    {
        return T::GetMetaInfoTypeName();
    }

    template <typename T>
    constexpr TypeMetaInfoID GetTypeMetaInfoID()
    {
        return T::GetMetaInfoID();
    }

    constexpr TypeMetaInfoID GetTypeMetaInfoID(const char* _typeName)
    {
        return _typeName ? Crc32(_typeName) : InvalidTypeMetaInfoID;
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
        return reinterpret_cast<uint8_t*>(static_cast<Base*>(reinterpret_cast<Type*>(0x10000))) - reinterpret_cast<uint8_t*>(reinterpret_cast<Type*>(0x10000));
    }

    template <typename Type, typename... Bases>
    TypeMetaInfo RegistryTypeMetaInfo()
    {
        return std::move(TypeMetaInfo(GetTypeMetaInfoID<Type>(), sizeof(Type), {{&(GetTypeMetaInfo<Bases>()), GetBaseTypeOffset<Type, Bases>()}...}));
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

#define EGO_RTTI(_TYPE, ...)                                                                                                                                                       \
    static const char* GetMetaInfoTypeName()                                                                                                                                       \
    {                                                                                                                                                                              \
        return #_TYPE;                                                                                                                                                             \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    static ego::rtti::TypeMetaInfoID GetMetaInfoID()                                                                                                                               \
    {                                                                                                                                                                              \
        static constexpr ego::rtti::TypeMetaInfoID ID = ego::rtti::GetTypeMetaInfoID(#_TYPE);                                                                                      \
        return ID;                                                                                                                                                                 \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    static const ego::rtti::TypeMetaInfo& GetMetaInfo()                                                                                                                            \
    {                                                                                                                                                                              \
        static const ego::rtti::TypeMetaInfo TypeMetaInfo = std::move(ego::rtti::RegistryTypeMetaInfo<_TYPE __VA_OPT__(, ) __VA_ARGS__>());                                        \
        return TypeMetaInfo;                                                                                                                                                       \
    }

#define EGO_RTTI_OBJECT(_PRE_MODIFICATOR, _POST_MODIFICATOR)                                                                                                                       \
    _PRE_MODIFICATOR const char* getObjectTypeInfoName() const _POST_MODIFICATOR                                                                                                   \
    {                                                                                                                                                                              \
        return GetMetaInfoTypeName();                                                                                                                                              \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    _PRE_MODIFICATOR ego::rtti::TypeMetaInfoID getObjectTypeMetaInfoID() const _POST_MODIFICATOR                                                                                   \
    {                                                                                                                                                                              \
        return GetMetaInfoID();                                                                                                                                                    \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    _PRE_MODIFICATOR const ego::rtti::TypeMetaInfo& getObjectTypeMetaInfo() const _POST_MODIFICATOR                                                                                \
    {                                                                                                                                                                              \
        return GetMetaInfo();                                                                                                                                                      \
    }

#define EGO_RTTI_VIRTUAL(_TYPE, ...)                                                                                                                                               \
    EGO_RTTI(_TYPE, __VA_ARGS__)                                                                                                                                                   \
    EGO_RTTI_OBJECT(virtual, override)

#define EGO_RTTI_VIRTUAL_BASE(_TYPE)                                                                                                                                               \
    EGO_RTTI(_TYPE)                                                                                                                                                                \
    EGO_RTTI_OBJECT(virtual, )

#define EGO_RTTI_TYPE_NAME(_TYPE) (ego::rtti::GetTypeMetaInfoName<_TYPE>())
#define EGO_RTTI_TYPE_ID(_TYPE) (ego::rtti::GetTypeMetaInfoID<_TYPE>())
#define EGO_RTTI_TYPE_META_INFO(_TYPE) (ego::rtti::GetTypeMetaInfo<_TYPE>())
