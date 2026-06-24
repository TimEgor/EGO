#include <utility>

namespace ego
{
    template <typename ValType, typename IndexType>
    IndexObjectPool<ValType, IndexType>::IndexObjectPool(IndexObjectPool&& _pool)
        : m_storage(std::move(_pool.m_storage))
    {
    }

    template <typename ValType, typename IndexType>
    bool IndexObjectPool<ValType, IndexType>::init(size_t _pageSize, size_t _maxFreePageCount, size_t _minFreeIndexCount)
    {
        return m_storage.init(_pageSize, _maxFreePageCount, _minFreeIndexCount);
    }

    template <typename ValType, typename IndexType>
    void IndexObjectPool<ValType, IndexType>::release()
    {
        m_storage.release();
    }

    template <typename ValType, typename IndexType>
    void IndexObjectPool<ValType, IndexType>::clear()
    {
        m_storage.clear();
    }

    template <typename ValType, typename IndexType>
    bool IndexObjectPool<ValType, IndexType>::isValid(IndexType _index) const
    {
        return m_storage.isValid(_index);
    }

    template <typename ValType, typename IndexType>
    const ValType* IndexObjectPool<ValType, IndexType>::getElement(IndexType _index) const
    {
        return m_storage.getElement(_index);
    }

    template <typename ValType, typename IndexType>
    ValType* IndexObjectPool<ValType, IndexType>::getElement(IndexType _index)
    {
        return m_storage.getElement(_index);
    }

    template <typename ValType, typename IndexType>
    void IndexObjectPool<ValType, IndexType>::addElementRaw(NewElementInfo& _info)
    {
        _info = makeElementInfo(m_storage.addElementRaw());
    }

    template <typename ValType, typename IndexType>
    typename IndexObjectPool<ValType, IndexType>::NewElementInfo IndexObjectPool<ValType, IndexType>::addElementRaw()
    {
        return makeElementInfo(m_storage.addElementRaw());
    }

    template <typename ValType, typename IndexType>
    template <typename... Args>
    void IndexObjectPool<ValType, IndexType>::addElement(NewElementInfo& _info, Args&&... _args)
    {
        StorageNewElementInfo storageInfo;
        m_storage.addElement(storageInfo, std::forward<Args>(_args)...);
        _info = makeElementInfo(storageInfo);
    }

    template <typename ValType, typename IndexType>
    template <typename... Args>
    typename IndexObjectPool<ValType, IndexType>::NewElementInfo IndexObjectPool<ValType, IndexType>::addElement(Args&&... _args)
    {
        NewElementInfo info;
        addElement(info, std::forward<Args>(_args)...);

        return info;
    }

    template <typename ValType, typename IndexType>
    void IndexObjectPool<ValType, IndexType>::removeElement(IndexType _index)
    {
        m_storage.removeElement(_index);
    }

    template <typename ValType, typename IndexType>
    typename IndexObjectPool<ValType, IndexType>::NewElementInfo IndexObjectPool<ValType, IndexType>::makeElementInfo(const StorageNewElementInfo& _info)
    {
        return NewElementInfo{_info.m_handle, _info.m_elementPtr};
    }
} // namespace ego
