#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#include "MTCountable.h"

namespace ego
{
    template <typename T>
    class IntrusivePointer;

    template <typename T, typename... TArguments>
    IntrusivePointer<T> MakeIntrusive(TArguments&&... _arguments);

    template <typename T, typename TDeleter, typename... TArguments>
    IntrusivePointer<T> MakeIntrusiveWithDeleter(TArguments&&... _arguments);

    template <typename T>
    class IntrusivePointer
    {
        template <typename U>
        friend class IntrusivePointer;

    public:
        using ObjectType = T;

        IntrusivePointer() = default;
        IntrusivePointer(std::nullptr_t);
        IntrusivePointer(T* _object);
        IntrusivePointer(const IntrusivePointer& _pointer);
        IntrusivePointer(IntrusivePointer&& _pointer) noexcept;

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        IntrusivePointer(const IntrusivePointer<U>& _pointer);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        IntrusivePointer(IntrusivePointer<U>&& _pointer) noexcept;

        ~IntrusivePointer();

        IntrusivePointer& operator=(std::nullptr_t);
        IntrusivePointer& operator=(T* _object);
        IntrusivePointer& operator=(const IntrusivePointer& _pointer);
        IntrusivePointer& operator=(IntrusivePointer&& _pointer) noexcept;

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        IntrusivePointer& operator=(const IntrusivePointer<U>& _pointer);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        IntrusivePointer& operator=(IntrusivePointer<U>&& _pointer) noexcept;

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
        void swap(IntrusivePointer& _pointer) noexcept;

    protected:
        static void validateIntrusivePointerType();

        void release();
        void assign(T* _object);

    protected:
        T* m_object = nullptr;
    };

    template <typename T1, typename T2>
    bool operator==(const IntrusivePointer<T1>& _pointer1, const IntrusivePointer<T2>& _pointer2);

    template <typename T1, typename T2>
    bool operator!=(const IntrusivePointer<T1>& _pointer1, const IntrusivePointer<T2>& _pointer2);
} // namespace ego

#define EGO_INTRUSIVE_POINTER_DECLARATION(_TYPE, _NAME, _POSTFIX) using _NAME##_POSTFIX = ego::IntrusivePointer<_TYPE>;

#define EGO_INTRUSIVE_POINTER(_TYPE) EGO_INTRUSIVE_POINTER_DECLARATION(_TYPE, _TYPE, Pointer)

#define EGO_NAMED_INTRUSIVE_POINTER(_TYPE, _NAME) EGO_INTRUSIVE_POINTER_DECLARATION(_TYPE, _NAME, Pointer)

#define EGO_MAKE_INTRUSIVE_POINTER(_TYPE, ...) ego::MakeIntrusive<_TYPE>(__VA_ARGS__)

#define EGO_MAKE_INTRUSIVE_POINTER_WITH_DELETER(_TYPE, _DELETER, ...) ego::MakeIntrusiveWithDeleter<_TYPE, _DELETER>(__VA_ARGS__)

#include "IntrusivePointer.hpp"
