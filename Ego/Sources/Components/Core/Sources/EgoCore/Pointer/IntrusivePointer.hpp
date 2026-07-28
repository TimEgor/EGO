template <typename T>
ego::IntrusivePointer<T>::IntrusivePointer(std::nullptr_t)
    : m_object(nullptr)
{
}

template <typename T>
void ego::IntrusivePointer<T>::validateIntrusivePointerType()
{
    static_assert(std::is_base_of_v<MTCountable, T>, "ego::IntrusivePointer<T> requires T to derive from ego::MTCountable");
}

template <typename T>
ego::IntrusivePointer<T>::IntrusivePointer(T* _object)
{
    validateIntrusivePointerType();
    assign(_object);
}

template <typename T>
ego::IntrusivePointer<T>::IntrusivePointer(const IntrusivePointer<T>& _pointer)
{
    validateIntrusivePointerType();
    assign(_pointer.m_object);
}

template <typename T>
ego::IntrusivePointer<T>::IntrusivePointer(IntrusivePointer<T>&& _pointer) noexcept
    : m_object(std::exchange(_pointer.m_object, nullptr))
{
    validateIntrusivePointerType();
}

template <typename T>
template <typename U, typename>
ego::IntrusivePointer<T>::IntrusivePointer(const ego::IntrusivePointer<U>& _pointer)
    : IntrusivePointer<T>(static_cast<T*>(_pointer.m_object))
{
}

template <typename T>
template <typename U, typename>
ego::IntrusivePointer<T>::IntrusivePointer(ego::IntrusivePointer<U>&& _pointer) noexcept
    : m_object(static_cast<T*>(std::exchange(_pointer.m_object, nullptr)))
{
    validateIntrusivePointerType();
}

template <typename T>
ego::IntrusivePointer<T>::~IntrusivePointer()
{
    reset();
}

template <typename T>
ego::IntrusivePointer<T>& ego::IntrusivePointer<T>::operator=(std::nullptr_t)
{
    reset();
    return *this;
}

template <typename T>
ego::IntrusivePointer<T>& ego::IntrusivePointer<T>::operator=(T* _object)
{
    assign(_object);
    return *this;
}

template <typename T>
ego::IntrusivePointer<T>& ego::IntrusivePointer<T>::operator=(const IntrusivePointer<T>& _pointer)
{
    if (this != &_pointer)
    {
        assign(_pointer.m_object);
    }

    return *this;
}

template <typename T>
ego::IntrusivePointer<T>& ego::IntrusivePointer<T>::operator=(IntrusivePointer<T>&& _pointer) noexcept
{
    if (this != &_pointer)
    {
        release();
        m_object = std::exchange(_pointer.m_object, nullptr);
    }

    return *this;
}

template <typename T>
template <typename U, typename>
ego::IntrusivePointer<T>& ego::IntrusivePointer<T>::operator=(const ego::IntrusivePointer<U>& _pointer)
{
    assign(static_cast<T*>(_pointer.m_object));
    return *this;
}

template <typename T>
template <typename U, typename>
ego::IntrusivePointer<T>& ego::IntrusivePointer<T>::operator=(ego::IntrusivePointer<U>&& _pointer) noexcept
{
    release();
    m_object = static_cast<T*>(std::exchange(_pointer.m_object, nullptr));
    return *this;
}

template <typename T>
void ego::IntrusivePointer<T>::reset()
{
    release();
}

template <typename T>
void ego::IntrusivePointer<T>::release()
{
    validateIntrusivePointerType();

    if (m_object)
    {
        T* object = m_object;
        m_object = nullptr;
        object->releaseReference();
    }
}

template <typename T>
void ego::IntrusivePointer<T>::assign(T* _object)
{
    validateIntrusivePointerType();

    if (m_object == _object)
    {
        return;
    }

    if (_object)
    {
        _object->addReference();
    }

    release();
    m_object = _object;
}

template <typename T>
void ego::IntrusivePointer<T>::swap(IntrusivePointer& _pointer) noexcept
{
    std::swap(m_object, _pointer.m_object);
}

template <typename T, typename... TArguments>
ego::IntrusivePointer<T> ego::MakeIntrusive(TArguments&&... _arguments)
{
    return MakeIntrusiveWithDeleter<T, std::default_delete<T>>(std::forward<TArguments>(_arguments)...);
}

template <typename T, typename TDeleter, typename... TArguments>
ego::IntrusivePointer<T> ego::MakeIntrusiveWithDeleter(TArguments&&... _arguments)
{
    static_assert(std::is_empty_v<TDeleter>);
    static_assert(std::is_default_constructible_v<TDeleter>);

    T* object = new T(std::forward<TArguments>(_arguments)...);
    object->setDestroyFunction(
        [](MTCountable* _object) noexcept
        {
            TDeleter{}(static_cast<T*>(_object));
        });

    return IntrusivePointer<T>(object);
}

template <typename T1, typename T2>
bool ego::operator==(const IntrusivePointer<T1>& _pointer1, const IntrusivePointer<T2>& _pointer2)
{
    return static_cast<const void*>(_pointer1.getObject()) == static_cast<const void*>(_pointer2.getObject());
}

template <typename T1, typename T2>
bool ego::operator!=(const IntrusivePointer<T1>& _pointer1, const IntrusivePointer<T2>& _pointer2)
{
    return !(_pointer1 == _pointer2);
}

template <typename T1, typename T2>
bool ego::operator<(const IntrusivePointer<T1>& _pointer1, const IntrusivePointer<T2>& _pointer2)
{
    return std::less<const void*>()(static_cast<const void*>(_pointer1.getObject()), static_cast<const void*>(_pointer2.getObject()));
}

template <typename T1, typename T2>
bool ego::operator>(const IntrusivePointer<T1>& _pointer1, const IntrusivePointer<T2>& _pointer2)
{
    return _pointer2 < _pointer1;
}

template <typename T1, typename T2>
bool ego::operator<=(const IntrusivePointer<T1>& _pointer1, const IntrusivePointer<T2>& _pointer2)
{
    return !(_pointer2 < _pointer1);
}

template <typename T1, typename T2>
bool ego::operator>=(const IntrusivePointer<T1>& _pointer1, const IntrusivePointer<T2>& _pointer2)
{
    return !(_pointer1 < _pointer2);
}
