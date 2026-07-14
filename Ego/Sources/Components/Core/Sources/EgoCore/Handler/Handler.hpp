#include "EgoCore/Assert/Assert.h"

namespace ego::handler_details
{
    inline std::nullptr_t ResolveHandlerObject(std::nullptr_t _object)
    {
        return _object;
    }

    template <typename T>
    T* ResolveHandlerObject(T* _object)
    {
        return _object;
    }

    template <typename T>
    const Reference<T>& ResolveHandlerObject(const Reference<T>& _object)
    {
        return _object;
    }

    template <typename T>
    const SharedPointer<T>& ResolveHandlerObject(const SharedPointer<T>& _object)
    {
        return _object;
    }

    template <typename T>
    T ResolveHandlerObject(const T& _object)
    {
        return _object;
    }

    template <typename T>
    typename Handler<T>::ObjectResult ResolveHandlerObject(const Handler<T>& _object)
    {
        return _object.getObject();
    }

    template <typename TResult, typename TValue>
    TResult ResolveHandlerObjectResult(TValue&& _value)
    {
        using ResolvedObjectType = decltype(ResolveHandlerObject(std::forward<TValue>(_value)));
        static_assert(std::is_convertible_v<ResolvedObjectType, TResult>);

        return ResolveHandlerObject(std::forward<TValue>(_value));
    }

    template <typename T>
    bool IsObjectValid(const T& _object)
    {
        if constexpr (HasObjectAccessV<T>)
        {
            return static_cast<bool>(_object);
        }
        else
        {
            return true;
        }
    }

    template <typename T>
    T* GetObjectPointer(T* _object)
    {
        return _object;
    }

    template <typename T>
    T* GetObjectPointer(const Reference<T>& _object)
    {
        return _object.getObject();
    }

    template <typename T>
    T* GetObjectPointer(const SharedPointer<T>& _object)
    {
        return _object.getObject();
    }
} // namespace ego::handler_details

template <typename T>
ego::Handler<T>::Handler(std::nullptr_t)
    : m_source(nullptr)
{
}

template <typename T>
ego::Handler<T>::Handler(const SourceReference& _source)
    : m_source(_source)
{
}

template <typename T>
ego::Handler<T>::Handler(const ObjectResult& _object)
    : Handler(MakeHandler<ObjectResult>(_object).getSource())
{
}

template <typename T>
ego::Handler<T>::Handler(ObjectResult&& _object)
    : Handler(MakeHandler<ObjectResult>(std::move(_object)).getSource())
{
}

template <typename T>
ego::Handler<T>& ego::Handler<T>::operator=(std::nullptr_t)
{
    reset();
    return *this;
}

template <typename T>
ego::Handler<T>& ego::Handler<T>::operator=(const ObjectResult& _object)
{
    m_source = MakeHandler<ObjectResult>(_object).getSource();
    return *this;
}

template <typename T>
ego::Handler<T>& ego::Handler<T>::operator=(ObjectResult&& _object)
{
    m_source = MakeHandler<ObjectResult>(std::move(_object)).getSource();
    return *this;
}

template <typename T>
typename ego::Handler<T>::ObjectResult ego::Handler<T>::getObject() const
{
    return m_source ? m_source->getObject() : ObjectResult();
}

template <typename T>
const typename ego::Handler<T>::SourceReference& ego::Handler<T>::getSource() const
{
    return m_source;
}

template <typename T>
typename ego::Handler<T>::ObjectType& ego::Handler<T>::operator*() const
    requires ego::handler_details::HasObjectAccessV<T>
{
    ObjectResult object = getObject();
    ObjectType* objectPointer = handler_details::GetObjectPointer(object);
    EGO_ASSERT(objectPointer);
    return *objectPointer;
}

template <typename T>
typename ego::Handler<T>::ObjectType* ego::Handler<T>::operator->() const
    requires ego::handler_details::HasObjectAccessV<T>
{
    ObjectResult object = getObject();
    ObjectType* objectPointer = handler_details::GetObjectPointer(object);
    EGO_ASSERT(objectPointer);
    return objectPointer;
}

template <typename T>
bool ego::Handler<T>::isNull() const
{
    return !isValid();
}

template <typename T>
bool ego::Handler<T>::isValid() const
{
    return m_source && handler_details::IsObjectValid(getObject());
}

template <typename T>
ego::Handler<T>::operator bool() const
{
    return isValid();
}

template <typename T>
void ego::Handler<T>::reset()
{
    m_source = nullptr;
}

template <typename TResult, typename TOwner, typename TResolver>
template <typename TOwnerArg, typename TResolverArg>
ego::ResolvedHandlerSource<TResult, TOwner, TResolver>::ResolvedHandlerSource(TOwnerArg&& _owner, TResolverArg&& _resolver)
    : m_owner(std::forward<TOwnerArg>(_owner)),
      m_resolver(std::forward<TResolverArg>(_resolver))
{
}

template <typename TResult, typename TOwner, typename TResolver>
TResult ego::ResolvedHandlerSource<TResult, TOwner, TResolver>::getObject() const
{
    return handler_details::ResolveHandlerObjectResult<TResult>(std::invoke(m_resolver, m_owner));
}

template <typename TResult, typename TOwner, typename TResolver>
ego::Handler<TResult> ego::MakeHandler(TOwner&& _owner, TResolver&& _resolver)
{
    using SourceOwnerType = std::decay_t<TOwner>;
    using SourceResolverType = std::decay_t<TResolver>;
    using SourceType = ResolvedHandlerSource<TResult, SourceOwnerType, SourceResolverType>;

    return Handler<TResult>(typename Handler<TResult>::SourceReference(new SourceType(std::forward<TOwner>(_owner), std::forward<TResolver>(_resolver))));
}

template <typename TResult>
ego::Handler<TResult> ego::MakeHandler(const TResult& _object)
{
    return MakeHandler<TResult>(
        _object,
        [](const TResult& _storedObject) -> TResult
        {
            return _storedObject;
        });
}
