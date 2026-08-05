#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/RTTI/Type/TypeMetaInfoID.h"

namespace ego::rtti
{
    class PropertyMetaInfo;

    using PropertyMetaInfoPointer = std::unique_ptr<PropertyMetaInfo>;
    using PropertyMetaInfoCollection = std::vector<PropertyMetaInfoPointer>;

    class TypeMetaInfo final : public NonCopyable
    {
    public:
        struct ParentTypeMetaInfoContext final
        {
            ParentTypeMetaInfoContext();
            ParentTypeMetaInfoContext(const TypeMetaInfo* _info, size_t _offset);

            const TypeMetaInfo* m_info = nullptr;
            size_t m_offset = 0;
        };

        using ParentTypeMetaInfoCollection = std::vector<ParentTypeMetaInfoContext>;

        class PropertyIterator final
        {
        public:
            PropertyIterator() = default;
            explicit PropertyIterator(const TypeMetaInfo* _typeMetaInfo);

            const PropertyMetaInfo& operator*() const;
            PropertyIterator& operator++();
            PropertyIterator operator++(int);

            bool operator==(const PropertyIterator& _iterator) const;
            bool operator!=(const PropertyIterator& _iterator) const;

            void* getValueAddress(void* _object) const;
            const void* getValueAddress(const void* _object) const;

            template <typename Value>
            Value& getValue(void* _object) const;

            template <typename Value>
            const Value& getValue(const void* _object) const;

        private:
            void advanceToProperty();
            size_t getValueOffset() const;

            const TypeMetaInfo* m_typeMetaInfo = nullptr;
            size_t m_typeIndex = 0;
            size_t m_propertyIndex = 0;
        };

        class PropertyRange final
        {
        public:
            PropertyRange() = default;
            explicit PropertyRange(const TypeMetaInfo* _typeMetaInfo);

            PropertyIterator begin() const;
            PropertyIterator end() const;
            bool empty() const;

        private:
            const TypeMetaInfo* m_typeMetaInfo = nullptr;
        };

        TypeMetaInfo(
            TypeMetaInfoID _id,
            ParentTypeMetaInfoCollection&& _parentTypeMetaInfos = ParentTypeMetaInfoCollection(),
            PropertyMetaInfoCollection&& _propertyMetaInfos = PropertyMetaInfoCollection());
        ~TypeMetaInfo() override;

        TypeMetaInfoID getID() const;
        PropertyRange getProperties() const;

        bool isBasedOn(const TypeMetaInfo& _baseType) const;
        bool isBasedOn(TypeMetaInfoID _baseTypeID) const;
        void* castTo(void* _object, const TypeMetaInfo& _baseType) const;

    private:
        ParentTypeMetaInfoCollection m_parentTypeMetaInfos;
        PropertyMetaInfoCollection m_properties;
        TypeMetaInfoID m_id;
    };

    template <typename T>
    const char* GetTypeMetaInfoName();

    template <typename T>
    const TypeMetaInfo& GetTypeMetaInfo();

    template <typename Type, typename Base>
    size_t GetBaseTypeOffset();

    template <typename Type, typename... Bases>
    TypeMetaInfo RegistryTypeMetaInfo();

    template <typename Type, typename Base>
    bool IsBasedOn();

    template <typename Type>
    bool IsObjectBasedOn(TypeMetaInfoID _baseTypeID);

    template <typename Base, typename Type>
    bool IsObjectBasedOn(const Type& _object);

    template <typename Type>
    bool IsObjectBasedOn(const Type& _object, TypeMetaInfoID _baseTypeID);

    template <typename Type, typename Base>
    Type* CastTo(const Base* _object);
} // namespace ego::rtti

#include "EgoCore/RTTI/Type/TypeMetaInfo.hpp"
