inline ego::ControlBlockBase::ControlBlockBase()
    : m_sharedCount(1),
      m_weakCount(1)
{
}

template <typename TObject, typename TDeleter>
ego::ControlBlock<TObject, TDeleter>::ControlBlock(TObject* _object, TDeleter _deleter)
    : m_object(_object),
      m_deleter(std::move(_deleter))
{
}

template <typename TObject, typename TDeleter>
void ego::ControlBlock<TObject, TDeleter>::destroyObject()
{
    if (m_object)
    {
        m_deleter(m_object);
        m_object = nullptr;
    }
}

template <typename TObject, typename TDeleter>
void* ego::ControlBlock<TObject, TDeleter>::getObject()
{
    return m_object;
}

template <typename TObject, typename TDeleter>
const void* ego::ControlBlock<TObject, TDeleter>::getObject() const
{
    return m_object;
}

template <typename T>
ego::SharedPointer<T>::SharedPointer()
    : m_object(nullptr),
      m_controlBlock(nullptr)
{
}

template <typename T>
ego::SharedPointer<T>::SharedPointer(std::nullptr_t)
    : m_object(nullptr),
      m_controlBlock(nullptr)
{
}

template <typename T>
ego::SharedPointer<T>::SharedPointer(T* _object)
    : m_object(nullptr),
      m_controlBlock(nullptr)
{
    if (!_object)
    {
        return;
    }

    using TDefaultDeleter = std::default_delete<T>;
    m_controlBlock = new ControlBlock<T, TDefaultDeleter>(_object, TDefaultDeleter{});
    m_object = _object;

    initializeEnableSharedFromThis(_object);
}

template <typename T>
template <typename TDeleter>
ego::SharedPointer<T>::SharedPointer(T* _object, TDeleter _deleter)
    : m_object(nullptr),
      m_controlBlock(nullptr)
{
    if (!_object)
    {
        return;
    }

    using TStoredDeleter = std::decay_t<TDeleter>;
    m_controlBlock = new ControlBlock<T, TStoredDeleter>(_object, std::forward<TDeleter>(_deleter));
    m_object = _object;

    initializeEnableSharedFromThis(_object);
}

template <typename T>
ego::SharedPointer<T>::SharedPointer(const SharedPointer& _pointer)
    : m_object(_pointer.m_object),
      m_controlBlock(_pointer.m_controlBlock)
{
    incrementSharedCount();
}

template <typename T>
ego::SharedPointer<T>::SharedPointer(SharedPointer&& _pointer) noexcept
    : m_object(_pointer.m_object),
      m_controlBlock(_pointer.m_controlBlock)
{
    _pointer.m_object = nullptr;
    _pointer.m_controlBlock = nullptr;
}

template <typename T>
template <typename U, typename>
ego::SharedPointer<T>::SharedPointer(const SharedPointer<U>& _pointer)
    : m_object(static_cast<T*>(_pointer.m_object)),
      m_controlBlock(_pointer.m_controlBlock)
{
    incrementSharedCount();
}

template <typename T>
template <typename U, typename>
ego::SharedPointer<T>::SharedPointer(SharedPointer<U>&& _pointer) noexcept
    : m_object(static_cast<T*>(_pointer.m_object)),
      m_controlBlock(_pointer.m_controlBlock)
{
    _pointer.m_object = nullptr;
    _pointer.m_controlBlock = nullptr;
}

template <typename T>
ego::SharedPointer<T>::SharedPointer(T* _object, ControlBlockBase* _controlBlock, bool _incrementReference)
    : m_object(_object),
      m_controlBlock(_controlBlock)
{
    if (_incrementReference)
    {
        incrementSharedCount();
    }
}

template <typename T>
ego::SharedPointer<T>::~SharedPointer()
{
    releaseSharedCount();
}

template <typename T>
ego::SharedPointer<T>& ego::SharedPointer<T>::operator=(std::nullptr_t)
{
    reset();
    return *this;
}

template <typename T>
ego::SharedPointer<T>& ego::SharedPointer<T>::operator=(T* _object)
{
    if (m_object != _object)
    {
        reset(_object);
    }

    return *this;
}

template <typename T>
ego::SharedPointer<T>& ego::SharedPointer<T>::operator=(const SharedPointer& _pointer)
{
    if (this != &_pointer)
    {
        releaseSharedCount();
        m_object = _pointer.m_object;
        m_controlBlock = _pointer.m_controlBlock;
        incrementSharedCount();
    }

    return *this;
}

template <typename T>
ego::SharedPointer<T>& ego::SharedPointer<T>::operator=(SharedPointer&& _pointer) noexcept
{
    if (this != &_pointer)
    {
        SharedPointer temporary(std::move(_pointer));
        swap(temporary);
    }

    return *this;
}

template <typename T>
template <typename U, typename>
ego::SharedPointer<T>& ego::SharedPointer<T>::operator=(const SharedPointer<U>& _pointer)
{
    if (m_controlBlock != _pointer.m_controlBlock)
    {
        releaseSharedCount();
        m_object = static_cast<T*>(_pointer.m_object);
        m_controlBlock = _pointer.m_controlBlock;
        incrementSharedCount();
    }
    else
    {
        m_object = static_cast<T*>(_pointer.m_object);
    }

    return *this;
}

template <typename T>
template <typename U, typename>
ego::SharedPointer<T>& ego::SharedPointer<T>::operator=(SharedPointer<U>&& _pointer) noexcept
{
    SharedPointer temporary(std::move(_pointer));
    swap(temporary);

    return *this;
}

template <typename T>
ego::SharedPointer<T>::operator bool() const
{
    return m_object != nullptr;
}

template <typename T>
T* ego::SharedPointer<T>::get() const
{
    return m_object;
}

template <typename T>
T* ego::SharedPointer<T>::getObject() const
{
    return m_object;
}

template <typename T>
T& ego::SharedPointer<T>::operator*() const
{
    return *m_object;
}

template <typename T>
T* ego::SharedPointer<T>::operator->() const
{
    return m_object;
}

template <typename T>
uint32_t ego::SharedPointer<T>::getUsingCount() const
{
    return m_controlBlock ? m_controlBlock->m_sharedCount.load(std::memory_order_acquire) : 0u;
}

template <typename T>
void ego::SharedPointer<T>::reset()
{
    releaseSharedCount();
}

template <typename T>
void ego::SharedPointer<T>::reset(T* _object)
{
    SharedPointer temporary(_object);
    swap(temporary);
}

template <typename T>
template <typename TDeleter>
void ego::SharedPointer<T>::reset(T* _object, TDeleter _deleter)
{
    SharedPointer temporary(_object, std::forward<TDeleter>(_deleter));
    swap(temporary);
}

template <typename T>
void ego::SharedPointer<T>::swap(SharedPointer& _pointer)
{
    std::swap(m_object, _pointer.m_object);
    std::swap(m_controlBlock, _pointer.m_controlBlock);
}

template <typename T>
void ego::SharedPointer<T>::incrementSharedCount()
{
    if (m_controlBlock)
    {
        m_controlBlock->m_sharedCount.fetch_add(1, std::memory_order_relaxed);
    }
}

template <typename T>
void ego::SharedPointer<T>::releaseSharedCount()
{
    if (!m_controlBlock)
    {
        m_object = nullptr;
        return;
    }

    if (m_controlBlock->m_sharedCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
        m_controlBlock->destroyObject();

        if (m_controlBlock->m_weakCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            delete m_controlBlock;
        }
    }

    m_object = nullptr;
    m_controlBlock = nullptr;
}

template <typename T>
void ego::SharedPointer<T>::initializeEnableSharedFromThis(T* _object)
{
    if constexpr (requires { typename T::EnableSharedFromThisType; })
    {
        using TEnabled = typename T::EnableSharedFromThisType;
        if constexpr (std::is_convertible_v<T*, TEnabled*> && std::is_convertible_v<T*, EnableSharedFromThis<TEnabled>*>)
        {
            EnableSharedFromThis<TEnabled>* enabledObject = static_cast<EnableSharedFromThis<TEnabled>*>(_object);
            enabledObject->acceptOwner(m_controlBlock, static_cast<TEnabled*>(_object));
        }
    }
}

template <typename T>
ego::WeakPointer<T>::WeakPointer()
    : m_object(nullptr),
      m_controlBlock(nullptr)
{
}

template <typename T>
ego::WeakPointer<T>::WeakPointer(std::nullptr_t)
    : m_object(nullptr),
      m_controlBlock(nullptr)
{
}

template <typename T>
ego::WeakPointer<T>::WeakPointer(const SharedPointer<T>& _pointer)
    : m_object(_pointer.m_object),
      m_controlBlock(_pointer.m_controlBlock)
{
    incrementWeakCount();
}

template <typename T>
ego::WeakPointer<T>::WeakPointer(const WeakPointer& _pointer)
    : m_object(_pointer.m_object),
      m_controlBlock(_pointer.m_controlBlock)
{
    incrementWeakCount();
}

template <typename T>
ego::WeakPointer<T>::WeakPointer(WeakPointer&& _pointer) noexcept
    : m_object(_pointer.m_object),
      m_controlBlock(_pointer.m_controlBlock)
{
    _pointer.m_object = nullptr;
    _pointer.m_controlBlock = nullptr;
}

template <typename T>
template <typename U, typename>
ego::WeakPointer<T>::WeakPointer(const SharedPointer<U>& _pointer)
    : m_object(static_cast<T*>(_pointer.m_object)),
      m_controlBlock(_pointer.m_controlBlock)
{
    incrementWeakCount();
}

template <typename T>
template <typename U, typename>
ego::WeakPointer<T>::WeakPointer(const WeakPointer<U>& _pointer)
    : m_object(static_cast<T*>(_pointer.m_object)),
      m_controlBlock(_pointer.m_controlBlock)
{
    incrementWeakCount();
}

template <typename T>
template <typename U, typename>
ego::WeakPointer<T>::WeakPointer(WeakPointer<U>&& _pointer) noexcept
    : m_object(static_cast<T*>(_pointer.m_object)),
      m_controlBlock(_pointer.m_controlBlock)
{
    _pointer.m_object = nullptr;
    _pointer.m_controlBlock = nullptr;
}

template <typename T>
ego::WeakPointer<T>::~WeakPointer()
{
    releaseWeakCount();
}

template <typename T>
ego::WeakPointer<T>& ego::WeakPointer<T>::operator=(const WeakPointer& _pointer)
{
    if (this != &_pointer)
    {
        releaseWeakCount();
        m_object = _pointer.m_object;
        m_controlBlock = _pointer.m_controlBlock;
        incrementWeakCount();
    }

    return *this;
}

template <typename T>
ego::WeakPointer<T>& ego::WeakPointer<T>::operator=(WeakPointer&& _pointer) noexcept
{
    if (this != &_pointer)
    {
        WeakPointer temporary(std::move(_pointer));
        swap(temporary);
    }

    return *this;
}

template <typename T>
ego::WeakPointer<T>& ego::WeakPointer<T>::operator=(const SharedPointer<T>& _pointer)
{
    if (m_controlBlock != _pointer.m_controlBlock)
    {
        releaseWeakCount();
        m_object = _pointer.m_object;
        m_controlBlock = _pointer.m_controlBlock;
        incrementWeakCount();
    }
    else
    {
        m_object = _pointer.m_object;
    }

    return *this;
}

template <typename T>
template <typename U, typename>
ego::WeakPointer<T>& ego::WeakPointer<T>::operator=(const SharedPointer<U>& _pointer)
{
    if (m_controlBlock != _pointer.m_controlBlock)
    {
        releaseWeakCount();
        m_object = static_cast<T*>(_pointer.m_object);
        m_controlBlock = _pointer.m_controlBlock;
        incrementWeakCount();
    }
    else
    {
        m_object = static_cast<T*>(_pointer.m_object);
    }

    return *this;
}

template <typename T>
template <typename U, typename>
ego::WeakPointer<T>& ego::WeakPointer<T>::operator=(const WeakPointer<U>& _pointer)
{
    if (m_controlBlock != _pointer.m_controlBlock)
    {
        releaseWeakCount();
        m_object = static_cast<T*>(_pointer.m_object);
        m_controlBlock = _pointer.m_controlBlock;
        incrementWeakCount();
    }
    else
    {
        m_object = static_cast<T*>(_pointer.m_object);
    }

    return *this;
}

template <typename T>
template <typename U, typename>
ego::WeakPointer<T>& ego::WeakPointer<T>::operator=(WeakPointer<U>&& _pointer) noexcept
{
    WeakPointer temporary(std::move(_pointer));
    swap(temporary);

    return *this;
}

template <typename T>
void ego::WeakPointer<T>::reset()
{
    releaseWeakCount();
}

template <typename T>
void ego::WeakPointer<T>::swap(WeakPointer& _pointer)
{
    std::swap(m_object, _pointer.m_object);
    std::swap(m_controlBlock, _pointer.m_controlBlock);
}

template <typename T>
uint32_t ego::WeakPointer<T>::getUsingCount() const
{
    return m_controlBlock ? m_controlBlock->m_sharedCount.load(std::memory_order_acquire) : 0u;
}

template <typename T>
bool ego::WeakPointer<T>::isExpired() const
{
    return !m_controlBlock || m_controlBlock->m_sharedCount.load(std::memory_order_acquire) == 0u;
}

template <typename T>
ego::SharedPointer<T> ego::WeakPointer<T>::lock() const
{
    if (!m_controlBlock)
    {
        return SharedPointer<T>();
    }

    uint32_t count = m_controlBlock->m_sharedCount.load(std::memory_order_acquire);

    while (count != 0u)
    {
        if (m_controlBlock->m_sharedCount.compare_exchange_weak(count, count + 1, std::memory_order_acq_rel, std::memory_order_acquire))
        {
            return SharedPointer<T>(m_object, m_controlBlock, false);
        }
    }

    return SharedPointer<T>();
}

template <typename T>
void ego::WeakPointer<T>::incrementWeakCount()
{
    if (m_controlBlock)
    {
        m_controlBlock->m_weakCount.fetch_add(1, std::memory_order_relaxed);
    }
}

template <typename T>
void ego::WeakPointer<T>::releaseWeakCount()
{
    if (!m_controlBlock)
    {
        m_object = nullptr;
        return;
    }

    if (m_controlBlock->m_weakCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
        delete m_controlBlock;
    }

    m_object = nullptr;
    m_controlBlock = nullptr;
}

template <typename T>
void ego::EnableSharedFromThis<T>::acceptOwner(ControlBlockBase* _controlBlock, T* _object) const
{
    if (m_weakThis.m_controlBlock == nullptr)
    {
        m_weakThis.m_object = _object;
        m_weakThis.m_controlBlock = _controlBlock;

        if (m_weakThis.m_controlBlock)
        {
            m_weakThis.incrementWeakCount();
        }
    }
}

template <typename To, typename From>
ego::SharedPointer<To> ego::StaticPointerCast(const SharedPointer<From>& _pointer)
{
    To* object = static_cast<To*>(_pointer.m_object);
    return SharedPointer<To>(object, _pointer.m_controlBlock, true);
}

template <typename T>
ego::SharedPointer<T> ego::EnableSharedFromThis<T>::sharedFromThis()
{
    SharedPointer<T> pointer = m_weakThis.lock();

    if (!pointer)
    {
        return nullptr;
    }

    return pointer;
}

template <typename T>
ego::WeakPointer<T> ego::EnableSharedFromThis<T>::weakFromThis()
{
    return m_weakThis;
}
