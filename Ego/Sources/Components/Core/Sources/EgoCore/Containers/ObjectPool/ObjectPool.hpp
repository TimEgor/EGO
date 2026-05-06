#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Memory/Utils.h"
#include "EgoCore/UtilsMacros.h"

namespace ego
{
    template <typename ValType, typename HandleType>
    ObjectPool<ValType, HandleType>::Page::Page(Page&& _page)
        : m_mem(nullptr),
          m_valsMem(nullptr),
          m_versionMem(nullptr),
          m_aliveMem(nullptr),
          m_capacity(_page.m_capacity),
          m_size(_page.m_size)
    {
        std::swap(m_mem, _page.m_mem);
        std::swap(m_valsMem, _page.m_valsMem);
        std::swap(m_versionMem, _page.m_versionMem);
        std::swap(m_aliveMem, _page.m_aliveMem);
        _page.m_capacity = 0;
        _page.m_size = 0;
    }

    template <typename ValType, typename HandleType>
    void ObjectPool<ValType, HandleType>::Page::allocate(size_t _capacity)
    {
        size_t valueMemSize = Align(sizeof(ValType) * _capacity, sizeof(void*));
        size_t versionMemSize = Align(sizeof(VersionType) * _capacity, sizeof(void*));
        size_t aliveMemSize = sizeof(bool) * _capacity;

        size_t memSize = Align(valueMemSize + versionMemSize + aliveMemSize, sizeof(void*));
        uint8_t* mem = new uint8_t[memSize];

        m_mem = mem;
        m_valsMem = reinterpret_cast<ValType*>(mem);
        m_versionMem = reinterpret_cast<VersionType*>(mem + valueMemSize);
        m_aliveMem = reinterpret_cast<bool*>(mem + valueMemSize + versionMemSize);

        m_capacity = _capacity;
        m_size = 0;

        EGO_ASSERT(m_mem);
        EGO_ASSERT(_capacity);

        memset(m_versionMem, InvalidVersion, versionMemSize);
        memset(m_aliveMem, 0, aliveMemSize);
    }

    template <typename ValType, typename HandleType>
    void ObjectPool<ValType, HandleType>::Page::deallocate()
    {
        EGO_SAFE_DESTROY_ARRAY(m_mem);

        m_valsMem = nullptr;
        m_versionMem = nullptr;
        m_aliveMem = nullptr;
        m_capacity = 0;
        m_size = 0;
    }

    template <typename ValType, typename HandleType>
    bool ObjectPool<ValType, HandleType>::Page::isAllocated() const
    {
        return m_mem;
    }

    template <typename ValType, typename HandleType>
    bool ObjectPool<ValType, HandleType>::checkElementLocation(const ElementLocation& _location) const
    {
        return _location.m_elementIndex < m_pageSize;
    }

    template <typename ValType, typename HandleType>
    bool ObjectPool<ValType, HandleType>::checkElementLocationWithPage(const ElementLocation& _location) const
    {
        return checkElementLocation(_location) && _location.m_pageIndex<m_pages.size();
    }

    template <typename ValType, typename HandleType>
    typename ObjectPool<ValType, HandleType>::ElementLocation ObjectPool<ValType, HandleType>::getElementLocation(
        IndexType _index
    ) const
    {
        ElementLocation location;
        location.m_pageIndex = _index / m_pageSize;
        location.m_elementIndex = _index % m_pageSize;

        return location;
    }

    template <typename ValType, typename HandleType>
    ValType* ObjectPool<ValType, HandleType>::getElementInternal(HandleType _handle) const
    {
        EGO_ASSERT(m_pageSize != 0);

        VersionType version = _handle.getVersion();
        if (version == InvalidVersion)
        {
            return nullptr;
        }

        IndexType index = _handle.getIndex();
        ElementLocation elementLocation = getElementLocation(index);

        if (!checkElementLocationWithPage(elementLocation))
        {
            return nullptr;
        }

        const Page& page = m_pages[elementLocation.m_pageIndex];
        if (!page.isAllocated() || !page.m_aliveMem[elementLocation.m_elementIndex])
        {
            return nullptr;
        }

        VersionType lastElementVersion = page.m_versionMem[elementLocation.m_elementIndex];

        if (lastElementVersion != version)
        {
            return nullptr;
        }

        return &page.m_valsMem[elementLocation.m_elementIndex];
    }

    template <typename ValType, typename HandleType>
    ObjectPool<ValType, HandleType>::ObjectPool(ObjectPool&& _pool)
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

    template <typename ValType, typename HandleType>
    bool ObjectPool<ValType, HandleType>::init(size_t _pageSize, size_t _maxFreePageCount, size_t _minFreeIndexCount)
    {
        EGO_ASSERT(m_pageSize == 0);

        m_pageSize = _pageSize;
        m_maxFreePageCount = _maxFreePageCount;
        m_minFreeIndexCount = _minFreeIndexCount;

        m_pages.reserve(1);

        return true;
    }

    template <typename ValType, typename HandleType>
    void ObjectPool<ValType, HandleType>::clear()
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

    template <typename ValType, typename HandleType>
    bool ObjectPool<ValType, HandleType>::isValid(HandleType _handle) const
    {
        EGO_ASSERT(m_pageSize != 0);

        VersionType version = _handle.getVersion();
        if (version == InvalidVersion)
        {
            return false;
        }

        IndexType index = _handle.getIndex();
        ElementLocation elementLocation = getElementLocation(index);

        if (!checkElementLocationWithPage(elementLocation))
        {
            return false;
        }

        const Page& page = m_pages[elementLocation.m_pageIndex];
        if (!page.isAllocated() || !page.m_aliveMem[elementLocation.m_elementIndex])
        {
            return false;
        }

        VersionType lastElementVersion = page.m_versionMem[elementLocation.m_elementIndex];

        return lastElementVersion == version;
    }

    template <typename ValType, typename HandleType>
    void ObjectPool<ValType, HandleType>::addElementRaw(NewElementInfo& _info)
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

        Page * pagePtr = nullptr;

        size_t pagesCount = m_pages.size();
        if (pagesCount <= elementLocation.m_pageIndex)
        {
            if (elementLocation.m_pageIndex - pagesCount < 2)
            {
                pagePtr = &m_pages.emplace_back();
            }
        }
        else
        {
            pagePtr = &m_pages[elementLocation.m_pageIndex];
        }

        if (!pagePtr)
        {
            EGO_ASSERT_FAIL_MESSAGE("ObjectPool::addElement() : Page indicing invalidation.");
            _info = NewElementInfo();
            return;
        }

        Page & page = *pagePtr;
        if (!page.isAllocated())
        {
            page.allocate(m_pageSize);
        }

        EGO_ASSERT(page.m_size < page.m_capacity);

        bool& aliveState = page.m_aliveMem[elementLocation.m_elementIndex];
        if (aliveState)
        {
            EGO_ASSERT_FAIL_MESSAGE("ObjectPool::addElement() : Reusing alive element.");
            _info = NewElementInfo();
            return;
        }

        ++page.m_size;

        ValType* val = &page.m_valsMem[elementLocation.m_elementIndex];

        VersionType& version = page.m_versionMem[elementLocation.m_elementIndex];

        ++version;
        if (version == InvalidVersion)
        {
            ++version;
        }

        aliveState = true;

        _info = NewElementInfo{HandleType(index, version), val};
    }

    template <typename ValType, typename HandleType>
    typename ObjectPool<ValType, HandleType>::NewElementInfo ObjectPool<ValType, HandleType>::addElementRaw()
    {
        NewElementInfo info;
        addElementRaw(info);

        return info;
    }

    template <typename ValType, typename HandleType>
    template <typename... Args>
    void ObjectPool<ValType, HandleType>::addElement(NewElementInfo& _info, Args&&... _args)
    {
        addElementRaw(_info);
        new(_info.m_elementPtr) ValType(_args...);
    }

    template <typename ValType, typename HandleType>
    template <typename... Args>
    typename ObjectPool<ValType, HandleType>::NewElementInfo ObjectPool<ValType, HandleType>::addElement(
        Args&&... _args
    )
    {
        NewElementInfo info = addElementRaw();
        new(info.m_elementPtr) ValType(_args...);

        return info;
    }

    template <typename ValType, typename HandleType>
    void ObjectPool<ValType, HandleType>::removeElement(HandleType _handle)
    {
        EGO_ASSERT(m_pageSize != 0);

        VersionType version = _handle.getVersion();
        if (version == InvalidVersion)
        {
            return;
        }

        IndexType index = _handle.getIndex();
        ElementLocation elementLocation = getElementLocation(index);

        if (!checkElementLocationWithPage(elementLocation))
        {
            return;
        }

        Page & page = m_pages[elementLocation.m_pageIndex];
        if (!page.isAllocated())
        {
            return;
        }

        bool& aliveState = page.m_aliveMem[elementLocation.m_elementIndex];
        if (!aliveState)
        {
            return;
        }

        VersionType lastElementVersion = page.m_versionMem[elementLocation.m_elementIndex];
        if (lastElementVersion != version)
        {
            return;
        }

        if constexpr (!std::is_trivially_destructible<ValType>::value)
        {
            ValType* val = &page.m_valsMem[elementLocation.m_elementIndex];
            val->~ValType();
        }

        m_freeIndices.push_back(index);
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
