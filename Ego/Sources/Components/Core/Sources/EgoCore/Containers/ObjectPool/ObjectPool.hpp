#include <utility>

namespace ego
{
    template <typename ValType, typename HandleType>
    ObjectPool<ValType, HandleType>::ObjectPool(ObjectPool&& _pool)
        : m_storage(std::move(_pool.m_storage))
    {}

    template <typename ValType, typename HandleType>
    bool ObjectPool<ValType, HandleType>::init(size_t _pageSize, size_t _maxFreePageCount, size_t _minFreeIndexCount)
    {
        return m_storage.init(_pageSize, _maxFreePageCount, _minFreeIndexCount);
    }

    template <typename ValType, typename HandleType>
    void ObjectPool<ValType, HandleType>::release()
    {
        m_storage.release();
    }

    template <typename ValType, typename HandleType>
    void ObjectPool<ValType, HandleType>::clear()
    {
        m_storage.clear();
    }

    template <typename ValType, typename HandleType>
    bool ObjectPool<ValType, HandleType>::isValid(HandleType _handle) const
    {
        return m_storage.isValid(_handle);
    }

    template <typename ValType, typename HandleType>
    const ValType* ObjectPool<ValType, HandleType>::getElement(HandleType _handle) const
    {
        return m_storage.getElement(_handle);
    }

    template <typename ValType, typename HandleType>
    ValType* ObjectPool<ValType, HandleType>::getElement(HandleType _handle)
    {
        return m_storage.getElement(_handle);
    }

    template <typename ValType, typename HandleType>
    void ObjectPool<ValType, HandleType>::addElementRaw(NewElementInfo& _info)
    {
        _info = makeElementInfo(m_storage.addElementRaw());
    }

    template <typename ValType, typename HandleType>
    typename ObjectPool<ValType, HandleType>::NewElementInfo ObjectPool<ValType, HandleType>::addElementRaw()
    {
        return makeElementInfo(m_storage.addElementRaw());
    }

    template <typename ValType, typename HandleType>
    template <typename... Args>
    void ObjectPool<ValType, HandleType>::addElement(NewElementInfo& _info, Args&&... _args)
    {
        StorageNewElementInfo storageInfo;
        m_storage.addElement(storageInfo, std::forward<Args>(_args)...);
        _info = makeElementInfo(storageInfo);
    }

    template <typename ValType, typename HandleType>
    template <typename... Args>
    typename ObjectPool<ValType, HandleType>::NewElementInfo ObjectPool<ValType, HandleType>::addElement(
        Args&&... _args
    )
    {
        NewElementInfo info;
        addElement(info, std::forward<Args>(_args)...);

        return info;
    }

    template <typename ValType, typename HandleType>
    void ObjectPool<ValType, HandleType>::removeElement(HandleType _handle)
    {
        m_storage.removeElement(_handle);
    }

    template <typename ValType, typename HandleType>
    typename ObjectPool<ValType, HandleType>::NewElementInfo ObjectPool<ValType, HandleType>::makeElementInfo(
        const StorageNewElementInfo& _info
    )
    {
        return NewElementInfo{_info.m_handle, _info.m_elementPtr};
    }
}
