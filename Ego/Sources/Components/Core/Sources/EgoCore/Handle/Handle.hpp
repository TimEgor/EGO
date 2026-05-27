namespace ego::handle_details
{
    template <typename T>
    T* ResolveHandleObject(T* _object)
    {
        return _object;
    }

    template <typename T>
    T* ResolveHandleObject(const Reference<T>& _object)
    {
        return _object.getObject();
    }

    template <typename T>
    T* ResolveHandleObject(const SharedPointer<T>& _object)
    {
        return _object.get();
    }

    template <typename TObject, typename TResult>
    TObject* ResolveHandleObjectResult(TResult&& _result)
    {
        using ResolvedObjectType = decltype(ResolveHandleObject(std::forward<TResult>(_result)));
        static_assert(std::is_convertible_v<ResolvedObjectType, TObject*>);

        return ResolveHandleObject(std::forward<TResult>(_result));
    }
}

template <typename T>
ego::Handle<T>::Handle(std::nullptr_t)
    : m_source(nullptr)
{}

template <typename T>
ego::Handle<T>::Handle(const SourceReference& _source)
    : m_source(_source)
{}

template <typename T>
T* ego::Handle<T>::getObject() const
{
    return m_source ? m_source->getObject() : nullptr;
}

template <typename T>
const typename ego::Handle<T>::SourceReference& ego::Handle<T>::getSource() const
{
    return m_source;
}

template <typename T>
bool ego::Handle<T>::isValid() const
{
    return getObject() != nullptr;
}

template <typename T>
ego::Handle<T>::operator bool() const
{
    return isValid();
}

template <typename T>
void ego::Handle<T>::reset()
{
    m_source = nullptr;
}

template <typename TObject, typename TOwner, typename TResolver>
template <typename TOwnerArg, typename TResolverArg>
ego::ResolvedHandleSource<TObject, TOwner, TResolver>::ResolvedHandleSource(
    TOwnerArg&& _owner,
    TResolverArg&& _resolver
)
    : m_owner(std::forward<TOwnerArg>(_owner)),
      m_resolver(std::forward<TResolverArg>(_resolver))
{}

template <typename TObject, typename TOwner, typename TResolver>
TObject* ego::ResolvedHandleSource<TObject, TOwner, TResolver>::getObject() const
{
    return handle_details::ResolveHandleObjectResult<TObject>(std::invoke(m_resolver, m_owner));
}

template <typename TObject, typename TOwner, typename TResolver>
ego::Handle<TObject> ego::MakeHandle(TOwner&& _owner, TResolver&& _resolver)
{
    using SourceOwnerType = std::decay_t<TOwner>;
    using SourceResolverType = std::decay_t<TResolver>;
    using SourceType = ResolvedHandleSource<TObject, SourceOwnerType, SourceResolverType>;

    return Handle<TObject>(
        typename Handle<TObject>::SourceReference(
            new SourceType(
                std::forward<TOwner>(_owner),
                std::forward<TResolver>(_resolver)
            )
        )
    );
}

template <typename TObject>
ego::Handle<TObject> ego::MakeHandle(TObject* _object)
{
    return MakeHandle<TObject>(
        _object,
        [](TObject* _storedObject) -> TObject*
        {
            return _storedObject;
        }
    );
}

template <typename TObject>
ego::Handle<TObject> ego::MakeHandle(const Reference<TObject>& _object)
{
    return MakeHandle<TObject>(
        _object,
        [](const Reference<TObject>& _storedObject) -> const Reference<TObject>&
        {
            return _storedObject;
        }
    );
}

template <typename TObject>
ego::Handle<TObject> ego::MakeHandle(const SharedPointer<TObject>& _object)
{
    return MakeHandle<TObject>(
        _object,
        [](const SharedPointer<TObject>& _storedObject) -> const SharedPointer<TObject>&
        {
            return _storedObject;
        }
    );
}
