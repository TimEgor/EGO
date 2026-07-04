#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

#include "MTCountable.h"

namespace ego
{
    template <typename T>
    class Reference
    {
        template <typename U>
        friend class Reference;

    public:
        using ObjectType = T;

        Reference() = default;
        Reference(std::nullptr_t);
        Reference(T* _object);
        Reference(const Reference& _reference);
        Reference(Reference&& _reference) noexcept;

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        Reference(const Reference<U>& _reference);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        Reference(Reference<U>&& _reference) noexcept;

        ~Reference();

        Reference& operator=(std::nullptr_t);
        Reference& operator=(T* _object);
        Reference& operator=(const Reference& _reference);
        Reference& operator=(Reference&& _reference) noexcept;

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        Reference& operator=(const Reference<U>& _reference);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        Reference& operator=(Reference<U>&& _reference) noexcept;

        T* operator->() const
        {
            return m_object;
        }
        T& operator*() const
        {
            return *m_object;
        }

        explicit operator bool() const
        {
            return m_object != nullptr;
        }
        operator T*() const
        {
            return m_object;
        }

        bool isNull() const
        {
            return m_object == nullptr;
        }

        T* get() const
        {
            return m_object;
        }
        T* getObject() const
        {
            return m_object;
        }
        T& getObjectRef() const
        {
            return *m_object;
        }

        template <typename CastType>
        CastType* getObjectCast() const
        {
            return static_cast<CastType*>(getObject());
        }

        template <typename CastType>
        CastType& getObjectCastRef() const
        {
            return static_cast<CastType&>(getObjectRef());
        }

        void reset();
        void swap(Reference& _reference) noexcept;

    protected:
        static void validateReferenceType();

        void release();
        void assign(T* _object);

        T* m_object = nullptr;
    };

    template <typename T1, typename T2>
    bool operator==(const Reference<T1>& _reference1, const Reference<T2>& _reference2);

    template <typename T1, typename T2>
    bool operator!=(const Reference<T1>& _reference1, const Reference<T2>& _reference2);

    template <typename T1, typename T2>
    bool operator<(const Reference<T1>& _reference1, const Reference<T2>& _reference2);

    template <typename T1, typename T2>
    bool operator>(const Reference<T1>& _reference1, const Reference<T2>& _reference2);

    template <typename T1, typename T2>
    bool operator<=(const Reference<T1>& _reference1, const Reference<T2>& _reference2);

    template <typename T1, typename T2>
    bool operator>=(const Reference<T1>& _reference1, const Reference<T2>& _reference2);
} // namespace ego

#define EGO_REFERENCE_DECLARATION(_TYPE, _NAME, _POSTFIX) using _NAME##_POSTFIX = ego::Reference<_TYPE>;

#define EGO_REFERENCE(_TYPE) EGO_REFERENCE_DECLARATION(_TYPE, _TYPE, Reference)

#define EGO_NAMED_REFERENCE(_TYPE, _NAME) EGO_REFERENCE_DECLARATION(_TYPE, _NAME, Reference)

#include "Reference.hpp"
