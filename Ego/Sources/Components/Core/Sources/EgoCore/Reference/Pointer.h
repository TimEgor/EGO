#pragma once

#include <atomic>

namespace ego
{
    template <typename T>
    class SharedPointer;

    template <typename T>
    class WeakPointer;

    template <typename T>
    class EnableSharedFromThis;

    struct ControlBlockBase
    {
        std::atomic<uint32_t> m_sharedCount;
        std::atomic<uint32_t> m_weakCount;

        ControlBlockBase();
        virtual ~ControlBlockBase() = default;

        virtual void destroyObject() = 0;
        virtual void* getObject() = 0;
        virtual const void* getObject() const = 0;
    };

    template <typename TObject, typename TDeleter>
    struct ControlBlock final : ControlBlockBase
    {
        TObject* m_object;
        TDeleter m_deleter;

        ControlBlock(TObject* _object, TDeleter _deleter);

        void destroyObject() override;
        void* getObject() override;
        const void* getObject() const override;
    };

    template <typename T>
    class SharedPointer
    {
        template <typename U>
        friend class SharedPointer;

        template <typename U>
        friend class WeakPointer;

        template <typename U>
        friend class EnableSharedFromThis;

        template <typename To, typename From>
        friend SharedPointer<To> StaticPointerCast(const SharedPointer<From>& _pointer);

    public:
        using ObjectType = T;

        SharedPointer();
        SharedPointer(std::nullptr_t);

        explicit SharedPointer(T* _object);

        template <typename TDeleter>
        SharedPointer(T* _object, TDeleter _deleter);

        SharedPointer(const SharedPointer& _pointer);
        SharedPointer(SharedPointer&& _pointer);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        SharedPointer(const SharedPointer<U>& _pointer);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        SharedPointer(SharedPointer<U>&& _pointer);

        ~SharedPointer();

        SharedPointer& operator=(const SharedPointer& _pointer);
        SharedPointer& operator=(SharedPointer&& _pointer);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        SharedPointer& operator=(const SharedPointer<U>& _pointer);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        SharedPointer& operator=(SharedPointer<U>&& _pointer);

        explicit operator bool() const;

        T* get() const;
        T& operator*() const;
        T* operator->() const;

        uint32_t getUsingCount() const;

        void reset();
        void reset(T* _object);

        template <typename TDeleter>
        void reset(T* _object, TDeleter _deleter);

        void swap(SharedPointer& _pointer);

    private:
        SharedPointer(T* _object, ControlBlockBase* _controlBlock, bool _incrementReference);

        void incrementSharedCount();
        void releaseSharedCount();
        void initializeEnableSharedFromThis(T* _object);

        T* m_object;
        ControlBlockBase* m_controlBlock;
    };

    template <typename T>
    class WeakPointer
    {
        template <typename U>
        friend class WeakPointer;

        template <typename U>
        friend class EnableSharedFromThis;

    public:
        WeakPointer();
        WeakPointer(std::nullptr_t);

        WeakPointer(const SharedPointer<T>& _pointer);
        WeakPointer(const WeakPointer& _pointer);
        WeakPointer(WeakPointer&& _pointer);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        WeakPointer(const SharedPointer<U>& _pointer);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        WeakPointer(const WeakPointer<U>& _pointer);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        WeakPointer(WeakPointer<U>&& _pointer);

        ~WeakPointer();

        WeakPointer& operator=(const WeakPointer& _pointer);
        WeakPointer& operator=(WeakPointer&& _pointer);
        WeakPointer& operator=(const SharedPointer<T>& _pointer);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        WeakPointer& operator=(const SharedPointer<U>& _pointer);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        WeakPointer& operator=(const WeakPointer<U>& _pointer);

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
        WeakPointer& operator=(WeakPointer<U>&& _pointer);

        void reset();
        void swap(WeakPointer& _pointer);

        uint32_t getUsingCount() const;
        bool isExpired() const;

        SharedPointer<T> lock() const;

    private:
        void incrementWeakCount();
        void releaseWeakCount();

        T* m_object;
        ControlBlockBase* m_controlBlock;
    };

    template <typename T>
    class EnableSharedFromThis
    {
        template <typename U>
        friend class SharedPointer;

    public:
        SharedPointer<T> sharedFromThis();
        WeakPointer<T> weakFromThis();

    protected:
        constexpr EnableSharedFromThis() = default;
        EnableSharedFromThis(const EnableSharedFromThis&) {}
        EnableSharedFromThis& operator=(const EnableSharedFromThis&) { return *this; }
        ~EnableSharedFromThis() = default;

    private:
        mutable WeakPointer<T> m_weakThis;

        void acceptOwner(ControlBlockBase* _controlBlock, T* _object) const;
    };

    template <typename To, typename From>
    SharedPointer<To> StaticPointerCast(const SharedPointer<From>& _pointer);
}

#define EGO_POINTER_DECLARATION(_TYPE, _NAME, _POSTFIX)	\
	using _NAME##_POSTFIX = ego::SharedPointer<_TYPE>;

#define EGO_POINTER(_TYPE)	\
	EGO_POINTER_DECLARATION(_TYPE, _TYPE, Pointer)

#define EGO_NAMED_POINTER(_TYPE, _NAME)	\
	EGO_POINTER_DECLARATION(_TYPE, _NAME, Pointer)

#define EGO_WEAK_POINTER_DECLARATION(_TYPE, _NAME, _POSTFIX)	\
	using _NAME##_POSTFIX = ego::WeakPointer<_TYPE>;

#define EGO_WEAK_POINTER(_TYPE)	\
	EGO_WEAK_POINTER_DECLARATION(_TYPE, _TYPE, WeakPointer)

#define EGO_NAMED_WEAK_POINTER(_TYPE, _NAME)	\
	EGO_WEAK_POINTER_DECLARATION(_TYPE, _NAME, WeakPointer)

#include "Pointer.hpp"
