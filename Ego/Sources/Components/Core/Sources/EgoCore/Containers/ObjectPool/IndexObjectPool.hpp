#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Memory/Utils.h"
#include "EgoCore/UtilsMacros.h"

namespace ego
{
    template <typename ValType, typename IndexType>
    IndexObjectPool<ValType, IndexType>::Page::Page(Page&& _page)
        : m_mem(nullptr),
          m_valsMem(nullptr),
          m_aliveMem(nullptr),
          m_capacity(_page.m_capacity),
          m_size(_page.m_size)
    {
        std::swap(m_mem, _page.m_mem);
        std::swap(m_valsMem, _page.m_valsMem);
        std::swap(m_aliveMem, _page.m_aliveMem);
        _page.m_capacity = 0;
        _page.m_size = 0;
    }

    template <typename ValType, typename IndexType>
    void IndexObjectPool<ValType, IndexType>::Page::allocate(size_t _capacity)
    {
        size_t valueMemSize = Align(sizeof(ValType) * _capacity, sizeof(void*));
        size_t aliveMemSize = sizeof(bool) * _capacity;

        size_t memSize = Align(valueMemSize + aliveMemSize, sizeof(void*));
        uint8_t* mem = new uint8_t[memSize];

        m_mem = mem;
        m_valsMem = reinterpret_cast<ValType*>(mem);
        m_aliveMem = reinterpret_cast<bool*>(mem + valueMemSize);

        m_capacity = _capacity;
        m_size = 0;

        EGO_ASSERT(m_mem);
        EGO_ASSERT(_capacity);

        memset(m_aliveMem, 0, aliveMemSize);
    }

    template <typename ValType, typename IndexType>
    void IndexObjectPool<ValType, IndexType>::Page::deallocate()
    {
        EGO_SAFE_DESTROY_ARRAY(m_mem);

        m_valsMem = nullptr;
        m_aliveMem = nullptr;
        m_capacity = 0;
        m_size = 0;
    }

    template <typename ValType, typename IndexType>
    bool IndexObjectPool<ValType, IndexType>::Page::isAllocated() const
    {
        return m_mem;
    }

    template <typename ValType, typename IndexType>
    bool IndexObjectPool<ValType, IndexType>::checkElementLocation(const ElementLocation& _location) const
    {
        return _location.elementIndex < m_pageSize;
    }

    template <typename ValType, typename IndexType>
    bool IndexObjectPool<ValType, IndexType>::checkElementLocationWithPage(const ElementLocation& _location) const
    {
        return checkElementLocation(_location) && _location.pageIndex < m_pages.size();
    }

    template <typename ValType, typename IndexType>
    typename IndexObjectPool<ValType, IndexType>::ElementLocation IndexObjectPool<
        ValType, IndexType>::getElementLocation(IndexType _index) const
    {
        ElementLocation location;
        location.pageIndex = _index / m_pageSize;
        location.elementIndex = _index % m_pageSize;

        return location;
    }

    template <typename ValType, typename IndexType>
    ValType* IndexObjectPool<ValType, IndexType>::getElementInternal(IndexType _index)
    {
        EGO_ASSERT(m_pageSize != 0);

        ElementLocation elementLocation = getElementLocation(_index);

        if (!checkElementLocationWithPage(elementLocation))
        {
            return nullptr;
        }

        const Page& page = m_pages[elementLocation.pageIndex];
        if (!page.isAllocated() || !page.m_aliveMem[elementLocation.elementIndex])
        {
            return nullptr;
        }

        return &page.m_valsMem[elementLocation.elementIndex];
    }

    template <typename ValType, typename IndexType>
    IndexObjectPool<ValType, IndexType>::IndexObjectPool(IndexObjectPool&& _pool)
        : m_size(_pool.m_size),
          m_emptyPages(_pool.m_emptyPages),
          m_maxUsedIndex(_pool.m_maxUsedIndex),
          m_pageSize(_pool.m_pageSize),
          m_maxFreePageCount(_pool.m_maxFreePageCount),
          m_minFreeIndexCount(_pool.m_minFreeIndexCount)
    {
        std::swap(m_pages, _pool.m_pages);
        std::swap(m_freeIndices, _pool.m_freeIndices);
        _pool.m_size = 0;
        _pool.m_emptyPages = 0;
        _pool.m_maxUsedIndex = 0;

        _pool.m_pageSize = 0;
        _pool.m_maxFreePageCount = 0;
        _pool.m_minFreeIndexCount = 0;
    }

    template <typename ValType, typename IndexType>
    bool IndexObjectPool<ValType, IndexType>::init(
        size_t _pageSize,
        size_t _maxFreePageCount,
        size_t _minFreeIndexCount
    )
    {
        EGO_ASSERT(m_pageSize == 0);

        m_pageSize = _pageSize;
        m_maxFreePageCount = _maxFreePageCount;
        m_minFreeIndexCount = _minFreeIndexCount;

        m_pages.reserve(1);

        return true;
    }

    template <typename ValType, typename IndexType>
    void IndexObjectPool<ValType, IndexType>::clear()
    {
        for (auto& page : m_pages)
        {
            if (!page.isAllocated())
            {
                continue;
            }

            for (size_t elementIndex = 0; elementIndex < page.m_capacity; ++elementIndex)
            {
                if (!page.m_aliveMem[elementIndex])
                {
                    continue;
                }

                if constexpr (!std::is_trivially_destructible<ValType>::value)
                {
                    ValType* val = &page.m_valsMem[elementIndex];
                    val->~ValType();
                }
            }

            page.deallocate();
        }
    }

    template <typename ValType, typename IndexType>
    bool IndexObjectPool<ValType, IndexType>::isValid(IndexType _index) const
    {
        EGO_ASSERT(m_pageSize != 0);

        ElementLocation elementLocation = getElementLocation(_index);

        if (!checkElementLocationWithPage(elementLocation))
        {
            return false;
        }

        const Page& page = m_pages[elementLocation.pageIndex];
        if (!page.isAllocated() || !page.m_aliveMem[elementLocation.elementIndex])
        {
            return false;
        }

        return true;
    }

    template <typename ValType, typename IndexType>
    void IndexObjectPool<ValType, IndexType>::addElementRaw(NewElementInfo& _info)
    {
        EGO_ASSERT(m_pageSize != 0);

        IndexType index;

        if (m_freeIndices.size() > m_minFreeIndexCount)
        {
            index = m_freeIndices.front();
            m_freeIndices.pop_front();
        }
        else
        {
            index = m_maxUsedIndex;
            ++m_maxUsedIndex;
        }

        ElementLocation elementLocation = getElementLocation(index);
        if (!checkElementLocation(elementLocation))
        {
            EGO_ASSERT_FAIL_MESSAGE("ObjectPool::addElement() : Element indicing invalidation.");
            _info = NewElementInfo();
            return;
        }

        Page* pagePtr = nullptr;

        size_t pagesCount = m_pages.size();
        if (pagesCount <= elementLocation.pageIndex)
        {
            if (elementLocation.pageIndex - pagesCount < 2)
            {
                pagePtr = &m_pages.emplace_back();
            }
        }
        else
        {
            pagePtr = &m_pages[elementLocation.pageIndex];
        }

        if (!pagePtr)
        {
            EGO_ASSERT_FAIL_MESSAGE("ObjectPool::addElement() : Page indicing invalidation.");
            _info = NewElementInfo();
            return;
        }

        Page& page = *pagePtr;
        if (!page.isAllocated())
        {
            page.allocate(m_pageSize);
        }
        EDGE_ASSERT(page.m_size < page.m_capacity);

        bool& aliveState = page.m_aliveMem[elementLocation.elementIndex];
        if (aliveState)
        {
            EGO_ASSERT_FAIL_MESSAGE("ObjectPool::addElement() : Reusing alive element.");
            _info = NewElementInfo();
            return;
        }

        ++page.m_size;

        ValType* val = &page.m_valsMem[elementLocation.elementIndex];

        aliveState = true;

        _info = NewElementInfo{index, val};
    }

    template <typename ValType, typename IndexType>
    typename IndexObjectPool<ValType, IndexType>::NewElementInfo IndexObjectPool<ValType, IndexType>::addElementRaw()
    {
        NewElementInfo info;
        addElementRaw(info);

        return info;
    }

    template <typename ValType, typename IndexType>
    template <typename... Args>
    void IndexObjectPool<ValType, IndexType>::addElement(NewElementInfo& _info, Args&&... _args)
    {
        addElementRaw(_info);
        new(_info.m_elementPtr) ValType(_args...);
    }

    template <typename ValType, typename IndexType>
    template <typename... Args>
    typename IndexObjectPool<ValType, IndexType>::NewElementInfo
    IndexObjectPool<ValType, IndexType>::addElement(Args&&... _args)
    {
        NewElementInfo info = addElementRaw();
        new(info.m_elementPtr) ValType(_args...);

        return info;
    }

    template <typename ValType, typename IndexType>
    void IndexObjectPool<ValType, IndexType>::removeElement(IndexType _index)
    {
        EGO_ASSERT(m_pageSize != 0);

        ElementLocation elementLocation = getElementLocation(_index);

        if (!checkElementLocationWithPage(elementLocation))
        {
            return;
        }

        Page& page = m_pages[elementLocation.pageIndex];
        if (!page.isAllocated())
        {
            return;
        }

        bool& aliveState = page.m_aliveMem[elementLocation.elementIndex];
        if (!aliveState)
        {
            return;
        }

        if constexpr (!std::is_trivially_destructible<ValType>::value)
        {
            ValType* val = &page.m_valsMem[elementLocation.elementIndex];
            val->~ValType();
        }

        m_freeIndices.push_back(_index);
        aliveState = false;

        EGO_ASSERT(page.m_size > 0);

        --page.m_size;
        if (page.m_size == 0)
        {
            ++m_emptyPages;
            if (m_emptyPages > m_maxFreePageCount)
            {
                page.deallocate();
            }
        }
    }
}
