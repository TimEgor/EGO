#pragma once

#include "ObjectPoolStorage.h"

#include <cstddef>
#include <type_traits>

namespace ego
{
    template <typename ValType, typename IndexType>
    class IndexObjectPool final
    {
        static_assert(std::is_integral<IndexType>::value);

    public:
        static constexpr IndexType InvalidIndex = 0;

        struct NewElementInfo final
        {
            IndexType m_elementIndex = InvalidIndex;
            ValType* m_elementPtr = nullptr;
        };

        IndexObjectPool() = default;
        IndexObjectPool(const IndexObjectPool&) = delete;
        IndexObjectPool(IndexObjectPool&& _pool);
        ~IndexObjectPool() { release(); }

        IndexObjectPool& operator=(const IndexObjectPool&) = delete;
        IndexObjectPool& operator=(IndexObjectPool&&) = delete;

        bool init(size_t _pageSize = 4096, size_t _maxFreePageCount = 2, size_t _minFreeIndexCount = 64);
        void release();

        void clear();

        bool isValid(IndexType _index) const;

        const ValType* getElement(IndexType _index) const;
        ValType* getElement(IndexType _index);

        void addElementRaw(NewElementInfo& _info);

        NewElementInfo addElementRaw();

        template <typename... Args>
        void addElement(NewElementInfo& _info, Args&&... _args);

        template <typename... Args>
        NewElementInfo addElement(Args&&... _args);

        void removeElement(IndexType _index);

    private:
        using StoragePolicy = detail::IndexObjectPoolPolicy<IndexType>;
        using Storage = detail::ObjectPoolStorage<ValType, StoragePolicy>;
        using StorageNewElementInfo = typename Storage::NewElementInfo;

        static NewElementInfo makeElementInfo(const StorageNewElementInfo& _info);

        Storage m_storage;
    };
}

#include "IndexObjectPool.hpp"
