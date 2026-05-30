#pragma once

#include "ObjectPoolHandle.h"
#include "ObjectPoolStorage.h"

#include <cstddef>

namespace ego
{
    template <typename ValType, typename HandleType>
    class ObjectPool final
    {
    public:
        using KeyType = typename HandleType::KeyType;
        using IndexType = typename HandleType::IndexType;
        using VersionType = typename HandleType::VersionType;

        static constexpr KeyType InvalidKey = HandleType::InvalidKey;
        static constexpr VersionType InvalidVersion = HandleType::InvalidVersion;
        static constexpr HandleType InvalidHandle = HandleType(InvalidKey);

        using HandleElementType = HandleType;

        struct NewElementInfo final
        {
            HandleType m_elementHandle = InvalidHandle;
            ValType* m_elementPtr = nullptr;
        };

        ObjectPool() = default;
        ObjectPool(const ObjectPool&) = delete;
        ObjectPool(ObjectPool&& _pool);
        ~ObjectPool() { release(); }

        ObjectPool& operator=(const ObjectPool&) = delete;
        ObjectPool& operator=(ObjectPool&&) = delete;

        bool init(size_t _pageSize = 4096, size_t _maxFreePageCount = 2, size_t _minFreeIndexCount = 64);
        void release();

        void clear();

        bool isValid(HandleType _handle) const;

        const ValType* getElement(HandleType _handle) const;
        ValType* getElement(HandleType _handle);

        void addElementRaw(NewElementInfo& _info);

        NewElementInfo addElementRaw();

        template <typename... Args>
        void addElement(NewElementInfo& _info, Args&&... _args);

        template <typename... Args>
        NewElementInfo addElement(Args&&... _args);

        void removeElement(HandleType _handle);

    private:
        using StoragePolicy = detail::VersionedObjectPoolPolicy<HandleType>;
        using Storage = detail::ObjectPoolStorage<ValType, StoragePolicy>;
        using StorageNewElementInfo = typename Storage::NewElementInfo;

        static NewElementInfo makeElementInfo(const StorageNewElementInfo& _info);

        Storage m_storage;
    };
}

#include "ObjectPool.hpp"
