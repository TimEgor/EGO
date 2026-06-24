#pragma once

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Memory/Utils.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

namespace ego::detail
{
    template <typename HandleType>
    struct VersionedObjectPoolPolicy final
    {
        using HandleElementType = HandleType;
        using KeyType = typename HandleType::KeyType;
        using IndexType = typename HandleType::IndexType;
        using VersionType = typename HandleType::VersionType;

        static_assert(std::is_unsigned_v<VersionType>);

        static constexpr bool HasVersion = true;
        static constexpr KeyType InvalidKey = HandleType::InvalidKey;
        static constexpr VersionType InvalidVersion = HandleType::InvalidVersion;
        static constexpr HandleElementType InvalidHandle = HandleElementType(InvalidKey);
        static constexpr IndexType InitialIndex = 0;

        static bool isInvalid(HandleElementType _handle)
        {
            return _handle.getVersion() == InvalidVersion;
        }

        static IndexType getIndex(HandleElementType _handle)
        {
            return _handle.getIndex();
        }

        static IndexType getStorageIndex(HandleElementType _handle)
        {
            return getIndex(_handle);
        }

        static VersionType getVersion(HandleElementType _handle)
        {
            return _handle.getVersion();
        }

        static HandleElementType makeHandle(IndexType _index, VersionType _version)
        {
            return HandleElementType(_index, _version);
        }

        static bool isHandleConsistent(HandleElementType _handle, IndexType _index, VersionType _version)
        {
            return getIndex(_handle) == _index && getVersion(_handle) == _version;
        }

        static VersionType nextVersion(IndexType _index, VersionType _version)
        {
            if (!canRepresentIndex(_index))
            {
                return InvalidVersion;
            }

            VersionType nextVersion = _version >= HandleType::MaxVersion ? static_cast<VersionType>(1) : static_cast<VersionType>(_version + 1);

            if (nextVersion == InvalidVersion)
            {
                ++nextVersion;
            }

            return nextVersion <= HandleType::MaxVersion ? nextVersion : InvalidVersion;
        }

        static bool canRepresentIndex(IndexType _index)
        {
            return HandleType::canRepresentIndex(_index);
        }
    };

    template <typename PoolIndexType>
    struct IndexObjectPoolPolicy final
    {
        static_assert(std::is_integral_v<PoolIndexType>);

        using HandleElementType = PoolIndexType;
        using IndexType = PoolIndexType;
        using VersionType = uint8_t;

        static constexpr bool HasVersion = false;
        static constexpr HandleElementType InvalidHandle = 0;
        static constexpr IndexType InitialIndex = 0;

        static bool isInvalid(HandleElementType _handle)
        {
            return _handle == InvalidHandle;
        }

        static IndexType getIndex(HandleElementType _handle)
        {
            return _handle;
        }

        static IndexType getStorageIndex(HandleElementType _handle)
        {
            return static_cast<IndexType>(_handle - 1);
        }

        static VersionType getVersion(HandleElementType)
        {
            return VersionType();
        }

        static HandleElementType makeHandle(IndexType _index, VersionType)
        {
            return static_cast<HandleElementType>(_index + 1);
        }

        static bool isHandleConsistent(HandleElementType _handle, IndexType _index, VersionType)
        {
            return !isInvalid(_handle) && getStorageIndex(_handle) == _index;
        }

        static VersionType nextVersion(IndexType, VersionType)
        {
            return VersionType();
        }

        static bool canRepresentIndex(IndexType _index)
        {
            if (_index == std::numeric_limits<IndexType>::max())
            {
                return false;
            }

            const HandleElementType handle = makeHandle(_index, VersionType());
            return isHandleConsistent(handle, _index, VersionType());
        }
    };

    template <typename ValType, typename HandlePolicy>
    class ObjectPoolStorage final
    {
    public:
        using HandleElementType = typename HandlePolicy::HandleElementType;
        using IndexType = typename HandlePolicy::IndexType;
        using VersionType = typename HandlePolicy::VersionType;

        struct NewElementInfo final
        {
            HandleElementType m_handle = HandlePolicy::InvalidHandle;
            ValType* m_elementPtr = nullptr;
        };

        ObjectPoolStorage() = default;
        ObjectPoolStorage(const ObjectPoolStorage&) = delete;
        ObjectPoolStorage(ObjectPoolStorage&& _pool) noexcept;
        ~ObjectPoolStorage()
        {
            release();
        }

        ObjectPoolStorage& operator=(const ObjectPoolStorage&) = delete;
        ObjectPoolStorage& operator=(ObjectPoolStorage&&) = delete;

        bool init(size_t _pageSize, size_t _maxFreePageCount, size_t _minFreeIndexCount);
        void release();
        void clear();

        bool isValid(HandleElementType _handle) const;

        const ValType* getElement(HandleElementType _handle) const;
        ValType* getElement(HandleElementType _handle);

        void addElementRaw(NewElementInfo& _info);
        NewElementInfo addElementRaw();

        template <typename... Args>
        void addElement(NewElementInfo& _info, Args&&... _args);

        template <typename... Args>
        NewElementInfo addElement(Args&&... _args);

        void removeElement(HandleElementType _handle);

    private:
        struct Page final
        {
            static constexpr size_t MemoryAlignment = std::max(std::max(alignof(ValType), alignof(VersionType)), alignof(bool));

            void* m_mem = nullptr;
            ValType* m_valsMem = nullptr;
            VersionType* m_versionMem = nullptr;
            bool* m_aliveMem = nullptr;

            size_t m_capacity = 0;
            size_t m_size = 0;

            Page() = default;
            Page(const Page&) = delete;
            Page(Page&& _page) noexcept;
            ~Page()
            {
                deallocate();
            }

            Page& operator=(const Page&) = delete;
            Page& operator=(Page&&) = delete;

            void allocate(size_t _capacity);
            void deallocate();

            bool isAllocated() const;
            bool isAlive(size_t _elementIndex) const;
            void setAlive(size_t _elementIndex, bool _isAlive);

            ValType* getValue(size_t _elementIndex);
            const ValType* getValue(size_t _elementIndex) const;

            VersionType& getVersion(size_t _elementIndex);
            const VersionType& getVersion(size_t _elementIndex) const;
        };

        struct ElementLocation final
        {
            size_t m_pageIndex = static_cast<size_t>(-1);
            size_t m_elementIndex = static_cast<size_t>(-1);
        };

        struct ElementSlot final
        {
            Page* m_page = nullptr;
            ElementLocation m_location;
        };

        struct ConstElementSlot final
        {
            const Page* m_page = nullptr;
            ElementLocation m_location;
        };

        bool acquireStorageIndex(IndexType& _index);
        bool checkElementLocation(const ElementLocation& _location) const;
        bool checkElementLocationWithPage(const ElementLocation& _location) const;

        ElementLocation getElementLocation(IndexType _index) const;
        Page& getOrCreatePage(size_t _pageIndex);

        bool findAliveLocation(HandleElementType _handle, ElementLocation& _location) const;
        bool findAliveSlot(HandleElementType _handle, ElementSlot& _slot);
        bool findAliveSlot(HandleElementType _handle, ConstElementSlot& _slot) const;

        bool prepareSlotForInsert(IndexType _storageIndex, ElementSlot& _slot);
        bool makeHandleForSlot(IndexType _storageIndex, const ElementSlot& _slot, VersionType& _version, HandleElementType& _handle);
        void activateSlot(ElementSlot& _slot, VersionType _version);
        void removeSlot(HandleElementType _handle, ElementSlot& _slot);
        void releaseEmptyPage(Page& _page);
        void destroyAliveElements(Page& _page);
        void resetRuntimeState();

        std::vector<Page> m_pages;
        std::deque<IndexType> m_freeIndices;
        size_t m_size = 0;
        size_t m_emptyPages = 0;
        IndexType m_maxUsedIndex = HandlePolicy::InitialIndex;
        bool m_isIndexRangeExhausted = false;

        size_t m_pageSize = 0;
        size_t m_maxFreePageCount = 0;
        size_t m_minFreeIndexCount = 0;
    };

    template <typename ValType, typename HandlePolicy>
    ObjectPoolStorage<ValType, HandlePolicy>::Page::Page(Page&& _page) noexcept
        : m_mem(_page.m_mem),
          m_valsMem(_page.m_valsMem),
          m_versionMem(_page.m_versionMem),
          m_aliveMem(_page.m_aliveMem),
          m_capacity(_page.m_capacity),
          m_size(_page.m_size)
    {
        _page.m_mem = nullptr;
        _page.m_valsMem = nullptr;
        _page.m_versionMem = nullptr;
        _page.m_aliveMem = nullptr;
        _page.m_capacity = 0;
        _page.m_size = 0;
    }

    template <typename ValType, typename HandlePolicy>
    void ObjectPoolStorage<ValType, HandlePolicy>::Page::allocate(size_t _capacity)
    {
        EGO_ASSERT(_capacity != 0);

        const size_t valueMemSize = Align(sizeof(ValType) * _capacity, MemoryAlignment);
        size_t versionMemSize = 0;
        if constexpr (HandlePolicy::HasVersion)
        {
            versionMemSize = Align(sizeof(VersionType) * _capacity, MemoryAlignment);
        }

        const size_t aliveMemSize = sizeof(bool) * _capacity;
        const size_t memSize = Align(valueMemSize + versionMemSize + aliveMemSize, MemoryAlignment);

        auto mem = static_cast<uint8_t*>(::operator new(memSize));

        m_mem = mem;
        m_valsMem = reinterpret_cast<ValType*>(mem);
        m_versionMem = HandlePolicy::HasVersion ? reinterpret_cast<VersionType*>(mem + valueMemSize) : nullptr;
        m_aliveMem = reinterpret_cast<bool*>(mem + valueMemSize + versionMemSize);

        m_capacity = _capacity;
        m_size = 0;

        if constexpr (HandlePolicy::HasVersion)
        {
            std::fill_n(m_versionMem, _capacity, HandlePolicy::InvalidVersion);
        }
        std::fill_n(m_aliveMem, _capacity, false);
    }

    template <typename ValType, typename HandlePolicy>
    void ObjectPoolStorage<ValType, HandlePolicy>::Page::deallocate()
    {
        if (m_mem)
        {
            ::operator delete(m_mem);
        }

        m_mem = nullptr;
        m_valsMem = nullptr;
        m_versionMem = nullptr;
        m_aliveMem = nullptr;
        m_capacity = 0;
        m_size = 0;
    }

    template <typename ValType, typename HandlePolicy>
    bool ObjectPoolStorage<ValType, HandlePolicy>::Page::isAllocated() const
    {
        return m_mem != nullptr;
    }

    template <typename ValType, typename HandlePolicy>
    bool ObjectPoolStorage<ValType, HandlePolicy>::Page::isAlive(size_t _elementIndex) const
    {
        return m_aliveMem[_elementIndex];
    }

    template <typename ValType, typename HandlePolicy>
    void ObjectPoolStorage<ValType, HandlePolicy>::Page::setAlive(size_t _elementIndex, bool _isAlive)
    {
        m_aliveMem[_elementIndex] = _isAlive;
    }

    template <typename ValType, typename HandlePolicy>
    ValType* ObjectPoolStorage<ValType, HandlePolicy>::Page::getValue(size_t _elementIndex)
    {
        return &m_valsMem[_elementIndex];
    }

    template <typename ValType, typename HandlePolicy>
    const ValType* ObjectPoolStorage<ValType, HandlePolicy>::Page::getValue(size_t _elementIndex) const
    {
        return &m_valsMem[_elementIndex];
    }

    template <typename ValType, typename HandlePolicy>
    typename ObjectPoolStorage<ValType, HandlePolicy>::VersionType& ObjectPoolStorage<ValType, HandlePolicy>::Page::getVersion(size_t _elementIndex)
    {
        return m_versionMem[_elementIndex];
    }

    template <typename ValType, typename HandlePolicy>
    const typename ObjectPoolStorage<ValType, HandlePolicy>::VersionType& ObjectPoolStorage<ValType, HandlePolicy>::Page::getVersion(size_t _elementIndex) const
    {
        return m_versionMem[_elementIndex];
    }

    template <typename ValType, typename HandlePolicy>
    ObjectPoolStorage<ValType, HandlePolicy>::ObjectPoolStorage(ObjectPoolStorage&& _pool) noexcept
        : m_pages(std::move(_pool.m_pages)),
          m_freeIndices(std::move(_pool.m_freeIndices)),
          m_size(_pool.m_size),
          m_emptyPages(_pool.m_emptyPages),
          m_maxUsedIndex(_pool.m_maxUsedIndex),
          m_isIndexRangeExhausted(_pool.m_isIndexRangeExhausted),
          m_pageSize(_pool.m_pageSize),
          m_maxFreePageCount(_pool.m_maxFreePageCount),
          m_minFreeIndexCount(_pool.m_minFreeIndexCount)
    {
        _pool.resetRuntimeState();
        _pool.m_pageSize = 0;
        _pool.m_maxFreePageCount = 0;
        _pool.m_minFreeIndexCount = 0;
    }

    template <typename ValType, typename HandlePolicy>
    bool ObjectPoolStorage<ValType, HandlePolicy>::init(size_t _pageSize, size_t _maxFreePageCount, size_t _minFreeIndexCount)
    {
        EGO_ASSERT(m_pageSize == 0);
        EGO_ASSERT(_pageSize != 0);

        if (m_pageSize != 0 || _pageSize == 0)
        {
            return false;
        }

        m_pageSize = _pageSize;
        m_maxFreePageCount = _maxFreePageCount;
        m_minFreeIndexCount = _minFreeIndexCount;
        resetRuntimeState();

        m_pages.reserve(1);

        return true;
    }

    template <typename ValType, typename HandlePolicy>
    void ObjectPoolStorage<ValType, HandlePolicy>::release()
    {
        clear();

        m_pageSize = 0;
        m_maxFreePageCount = 0;
        m_minFreeIndexCount = 0;
    }

    template <typename ValType, typename HandlePolicy>
    void ObjectPoolStorage<ValType, HandlePolicy>::clear()
    {
        for (Page& page : m_pages)
        {
            destroyAliveElements(page);
        }

        m_pages.clear();
        m_freeIndices.clear();
        resetRuntimeState();
    }

    template <typename ValType, typename HandlePolicy>
    bool ObjectPoolStorage<ValType, HandlePolicy>::isValid(HandleElementType _handle) const
    {
        ElementLocation location;
        return findAliveLocation(_handle, location);
    }

    template <typename ValType, typename HandlePolicy>
    const ValType* ObjectPoolStorage<ValType, HandlePolicy>::getElement(HandleElementType _handle) const
    {
        ConstElementSlot slot;
        if (!findAliveSlot(_handle, slot))
        {
            return nullptr;
        }

        return slot.m_page->getValue(slot.m_location.m_elementIndex);
    }

    template <typename ValType, typename HandlePolicy>
    ValType* ObjectPoolStorage<ValType, HandlePolicy>::getElement(HandleElementType _handle)
    {
        ElementSlot slot;
        if (!findAliveSlot(_handle, slot))
        {
            return nullptr;
        }

        return slot.m_page->getValue(slot.m_location.m_elementIndex);
    }

    template <typename ValType, typename HandlePolicy>
    void ObjectPoolStorage<ValType, HandlePolicy>::addElementRaw(NewElementInfo& _info)
    {
        _info = NewElementInfo();

        EGO_ASSERT(m_pageSize != 0);
        if (m_pageSize == 0)
        {
            return;
        }

        IndexType index;
        if (!acquireStorageIndex(index))
        {
            return;
        }

        ElementSlot slot;
        if (!prepareSlotForInsert(index, slot))
        {
            return;
        }

        VersionType version = VersionType();
        HandleElementType handle = HandlePolicy::InvalidHandle;
        if (!makeHandleForSlot(index, slot, version, handle))
        {
            return;
        }

        activateSlot(slot, version);

        _info = NewElementInfo{handle, slot.m_page->getValue(slot.m_location.m_elementIndex)};
    }

    template <typename ValType, typename HandlePolicy>
    typename ObjectPoolStorage<ValType, HandlePolicy>::NewElementInfo ObjectPoolStorage<ValType, HandlePolicy>::addElementRaw()
    {
        NewElementInfo info;
        addElementRaw(info);

        return info;
    }

    template <typename ValType, typename HandlePolicy>
    template <typename... Args>
    void ObjectPoolStorage<ValType, HandlePolicy>::addElement(NewElementInfo& _info, Args&&... _args)
    {
        addElementRaw(_info);
        if (!_info.m_elementPtr)
        {
            return;
        }

        new (_info.m_elementPtr) ValType(std::forward<Args>(_args)...);
    }

    template <typename ValType, typename HandlePolicy>
    template <typename... Args>
    typename ObjectPoolStorage<ValType, HandlePolicy>::NewElementInfo ObjectPoolStorage<ValType, HandlePolicy>::addElement(Args&&... _args)
    {
        NewElementInfo info;
        addElement(info, std::forward<Args>(_args)...);

        return info;
    }

    template <typename ValType, typename HandlePolicy>
    void ObjectPoolStorage<ValType, HandlePolicy>::removeElement(HandleElementType _handle)
    {
        ElementSlot slot;
        if (!findAliveSlot(_handle, slot))
        {
            return;
        }

        removeSlot(_handle, slot);
    }

    template <typename ValType, typename HandlePolicy>
    bool ObjectPoolStorage<ValType, HandlePolicy>::acquireStorageIndex(IndexType& _index)
    {
        if (m_freeIndices.size() > m_minFreeIndexCount)
        {
            _index = m_freeIndices.front();
            m_freeIndices.pop_front();
            return true;
        }

        if (m_isIndexRangeExhausted)
        {
            EGO_ASSERT_FAIL_MESSAGE("ObjectPool::addElement() : Index overflow.");
            return false;
        }

        if (!HandlePolicy::canRepresentIndex(m_maxUsedIndex))
        {
            EGO_ASSERT_FAIL_MESSAGE("ObjectPool::addElement() : Index is not representable by handle.");
            m_isIndexRangeExhausted = true;
            return false;
        }

        _index = m_maxUsedIndex;
        if (m_maxUsedIndex == std::numeric_limits<IndexType>::max())
        {
            m_isIndexRangeExhausted = true;
        }
        else
        {
            ++m_maxUsedIndex;
        }

        return true;
    }

    template <typename ValType, typename HandlePolicy>
    bool ObjectPoolStorage<ValType, HandlePolicy>::checkElementLocation(const ElementLocation& _location) const
    {
        return _location.m_elementIndex < m_pageSize;
    }

    template <typename ValType, typename HandlePolicy>
    bool ObjectPoolStorage<ValType, HandlePolicy>::checkElementLocationWithPage(const ElementLocation& _location) const
    {
        return checkElementLocation(_location) && _location.m_pageIndex < m_pages.size();
    }

    template <typename ValType, typename HandlePolicy>
    typename ObjectPoolStorage<ValType, HandlePolicy>::ElementLocation ObjectPoolStorage<ValType, HandlePolicy>::getElementLocation(IndexType _index) const
    {
        const size_t index = static_cast<size_t>(_index);
        return ElementLocation{index / m_pageSize, index % m_pageSize};
    }

    template <typename ValType, typename HandlePolicy>
    typename ObjectPoolStorage<ValType, HandlePolicy>::Page& ObjectPoolStorage<ValType, HandlePolicy>::getOrCreatePage(size_t _pageIndex)
    {
        while (m_pages.size() <= _pageIndex)
        {
            m_pages.emplace_back();
        }

        return m_pages[_pageIndex];
    }

    template <typename ValType, typename HandlePolicy>
    bool ObjectPoolStorage<ValType, HandlePolicy>::findAliveLocation(HandleElementType _handle, ElementLocation& _location) const
    {
        EGO_ASSERT(m_pageSize != 0);
        if (m_pageSize == 0 || HandlePolicy::isInvalid(_handle))
        {
            return false;
        }

        _location = getElementLocation(HandlePolicy::getStorageIndex(_handle));
        if (!checkElementLocationWithPage(_location))
        {
            return false;
        }

        const Page& page = m_pages[_location.m_pageIndex];
        if (!page.isAllocated() || !page.isAlive(_location.m_elementIndex))
        {
            return false;
        }

        if constexpr (HandlePolicy::HasVersion)
        {
            if (page.getVersion(_location.m_elementIndex) != HandlePolicy::getVersion(_handle))
            {
                return false;
            }
        }

        return true;
    }

    template <typename ValType, typename HandlePolicy>
    bool ObjectPoolStorage<ValType, HandlePolicy>::findAliveSlot(HandleElementType _handle, ElementSlot& _slot)
    {
        ElementLocation location;
        if (!findAliveLocation(_handle, location))
        {
            _slot = ElementSlot();
            return false;
        }

        _slot = ElementSlot{&m_pages[location.m_pageIndex], location};
        return true;
    }

    template <typename ValType, typename HandlePolicy>
    bool ObjectPoolStorage<ValType, HandlePolicy>::findAliveSlot(HandleElementType _handle, ConstElementSlot& _slot) const
    {
        ElementLocation location;
        if (!findAliveLocation(_handle, location))
        {
            _slot = ConstElementSlot();
            return false;
        }

        _slot = ConstElementSlot{&m_pages[location.m_pageIndex], location};
        return true;
    }

    template <typename ValType, typename HandlePolicy>
    bool ObjectPoolStorage<ValType, HandlePolicy>::prepareSlotForInsert(IndexType _storageIndex, ElementSlot& _slot)
    {
        const ElementLocation location = getElementLocation(_storageIndex);
        if (!checkElementLocation(location))
        {
            EGO_ASSERT_FAIL_MESSAGE("ObjectPool::addElement() : Element indexing invalidation.");
            _slot = ElementSlot();
            return false;
        }

        Page* page = nullptr;
        if (location.m_pageIndex < m_pages.size())
        {
            page = &m_pages[location.m_pageIndex];
            if (page->isAllocated() && page->isAlive(location.m_elementIndex))
            {
                EGO_ASSERT_FAIL_MESSAGE("ObjectPool::addElement() : Reusing alive element.");
                _slot = ElementSlot();
                return false;
            }
        }

        _slot = ElementSlot{page, location};
        return true;
    }

    template <typename ValType, typename HandlePolicy>
    bool ObjectPoolStorage<ValType, HandlePolicy>::makeHandleForSlot(IndexType _storageIndex, const ElementSlot& _slot, VersionType& _version, HandleElementType& _handle)
    {
        _version = VersionType();
        if constexpr (HandlePolicy::HasVersion)
        {
            VersionType pageVersion = HandlePolicy::InvalidVersion;
            if (_slot.m_page && _slot.m_page->isAllocated())
            {
                pageVersion = _slot.m_page->getVersion(_slot.m_location.m_elementIndex);
            }

            _version = HandlePolicy::nextVersion(_storageIndex, pageVersion);
            if (_version == HandlePolicy::InvalidVersion)
            {
                EGO_ASSERT_FAIL_MESSAGE("ObjectPool::addElement() : Handle version overflow.");
                return false;
            }
        }

        _handle = HandlePolicy::makeHandle(_storageIndex, _version);
        if (!HandlePolicy::isHandleConsistent(_handle, _storageIndex, _version))
        {
            EGO_ASSERT_FAIL_MESSAGE("ObjectPool::addElement() : Handle indexing invalidation.");
            _handle = HandlePolicy::InvalidHandle;
            return false;
        }

        return true;
    }

    template <typename ValType, typename HandlePolicy>
    void ObjectPoolStorage<ValType, HandlePolicy>::activateSlot(ElementSlot& _slot, VersionType _version)
    {
        if (!_slot.m_page)
        {
            _slot.m_page = &getOrCreatePage(_slot.m_location.m_pageIndex);
        }

        if (!_slot.m_page->isAllocated())
        {
            _slot.m_page->allocate(m_pageSize);
        }
        else if (_slot.m_page->m_size == 0 && m_emptyPages > 0)
        {
            --m_emptyPages;
        }

        EGO_ASSERT(_slot.m_page->m_size < _slot.m_page->m_capacity);
        EGO_ASSERT(!_slot.m_page->isAlive(_slot.m_location.m_elementIndex));

        if constexpr (HandlePolicy::HasVersion)
        {
            _slot.m_page->getVersion(_slot.m_location.m_elementIndex) = _version;
        }

        _slot.m_page->setAlive(_slot.m_location.m_elementIndex, true);

        ++_slot.m_page->m_size;
        ++m_size;
    }

    template <typename ValType, typename HandlePolicy>
    void ObjectPoolStorage<ValType, HandlePolicy>::removeSlot(HandleElementType _handle, ElementSlot& _slot)
    {
        if constexpr (!std::is_trivially_destructible_v<ValType>)
        {
            ValType* val = _slot.m_page->getValue(_slot.m_location.m_elementIndex);
            val->~ValType();
        }

        m_freeIndices.push_back(HandlePolicy::getStorageIndex(_handle));

        _slot.m_page->setAlive(_slot.m_location.m_elementIndex, false);

        EGO_ASSERT(_slot.m_page->m_size > 0);
        EGO_ASSERT(m_size > 0);

        --_slot.m_page->m_size;
        --m_size;

        releaseEmptyPage(*_slot.m_page);
    }

    template <typename ValType, typename HandlePolicy>
    void ObjectPoolStorage<ValType, HandlePolicy>::releaseEmptyPage(Page& _page)
    {
        if (_page.m_size != 0)
        {
            return;
        }

        ++m_emptyPages;
        if (m_emptyPages > m_maxFreePageCount)
        {
            _page.deallocate();
            --m_emptyPages;
        }
    }

    template <typename ValType, typename HandlePolicy>
    void ObjectPoolStorage<ValType, HandlePolicy>::destroyAliveElements(Page& _page)
    {
        if (!_page.isAllocated())
        {
            return;
        }

        if constexpr (!std::is_trivially_destructible_v<ValType>)
        {
            for (size_t elementIndex = 0; elementIndex < _page.m_capacity; ++elementIndex)
            {
                if (!_page.isAlive(elementIndex))
                {
                    continue;
                }

                ValType* val = _page.getValue(elementIndex);
                val->~ValType();
            }
        }
    }

    template <typename ValType, typename HandlePolicy>
    void ObjectPoolStorage<ValType, HandlePolicy>::resetRuntimeState()
    {
        m_size = 0;
        m_emptyPages = 0;
        m_maxUsedIndex = HandlePolicy::InitialIndex;
        m_isIndexRangeExhausted = false;
    }
} // namespace ego::detail
