#pragma once

#include <type_traits>
#include <vector>
#include <deque>

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

        bool init(size_t _pageSize = 4096, size_t _maxFreePageCount = 2, size_t _minFreeIndexCount = 64);
        void release() { clear(); }

        void clear();

        bool isValid(IndexType _index) const;

        const ValType* getElement(IndexType _index) const { return getElementInternal(_index); }
        ValType* getElement(IndexType _index) { return getElementInternal(_index); }

        void addElementRaw(NewElementInfo& _info);

        NewElementInfo addElementRaw();

        template <typename... Args>
        void addElement(NewElementInfo& _info, Args&&... _args);

        template <typename... Args>
        NewElementInfo addElement(Args&&... _args);

        void removeElement(IndexType _index);

    private:
        struct Page final
        {
            void* m_mem = nullptr;
            ValType* m_valsMem = nullptr;
            bool* m_aliveMem = nullptr;

            size_t m_capacity = 0;
            size_t m_size = 0;

            Page() = default;
            Page(const Page&) = delete;
            Page(Page&& _page);

            ~Page() { deallocate(); }

            Page& operator=(const Page&) = delete;
            Page& operator=(Page&&) = delete;

            void allocate(size_t _capacity);
            void deallocate();

            bool isAllocated() const;
        };

        struct ElementLocation final
        {
            size_t m_pageIndex = -1;
            size_t m_elementIndex = -1;
        };

        bool checkElementLocation(const ElementLocation& _location) const;
        bool checkElementLocationWithPage(const ElementLocation& _location) const;

        ElementLocation getElementLocation(IndexType _index) const;
        ValType* getElementInternal(IndexType _index);

        std::vector<Page> m_pages;
        std::deque<IndexType> m_freeIndices;
        size_t m_size = 0;
        size_t m_emptyPages = 0;
        IndexType m_maxUsedIndex = 0;

        size_t m_pageSize = 0;
        size_t m_maxFreePageCount = 0;
        size_t m_minFreeIndexCount = 0;
    };
}

#include "IndexObjectPool.hpp"
