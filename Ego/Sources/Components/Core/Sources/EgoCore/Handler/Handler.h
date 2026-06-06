#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/Reference/Reference.h"

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace ego
{
    namespace handler_details
    {
        template <typename T>
        struct HandlerObjectType final
        {
            using Type = T;
        };

        template <typename T>
        struct HandlerObjectType<T*> final
        {
            using Type = T;
        };

        template <typename T>
        struct HandlerObjectType<Reference<T>> final
        {
            using Type = T;
        };

        template <typename T>
        struct HandlerObjectType<SharedPointer<T>> final
        {
            using Type = T;
        };

        template <typename T>
        using HandlerObjectTypeT = typename HandlerObjectType<std::remove_cv_t<T>>::Type;

        template <typename T>
        struct HasObjectAccess : std::false_type
        {};

        template <typename T>
        struct HasObjectAccess<T*> : std::true_type
        {};

        template <typename T>
        struct HasObjectAccess<Reference<T>> : std::true_type
        {};

        template <typename T>
        struct HasObjectAccess<SharedPointer<T>> : std::true_type
        {};

        template <typename T>
        inline constexpr bool HasObjectAccessV = HasObjectAccess<std::remove_cv_t<T>>::value;

        template <typename T>
        bool IsObjectValid(const T& _object);

        template <typename T>
        T* GetObjectPointer(T* _object);

        template <typename T>
        T* GetObjectPointer(const Reference<T>& _object);

        template <typename T>
        T* GetObjectPointer(const SharedPointer<T>& _object);
    }

    template <typename T>
    class HandlerSource : public STDDestroyMTCountable
    {
    public:
        virtual T getObject() const = 0;
    };

    template <typename T>
    class Handler
    {
    public:
        using ObjectResult = T;
        using ObjectType = handler_details::HandlerObjectTypeT<ObjectResult>;
        using SourceType = HandlerSource<ObjectResult>;
        using SourceReference = Reference<SourceType>;

        Handler() = default;
        Handler(std::nullptr_t);
        explicit Handler(const SourceReference& _source);
        Handler(const ObjectResult& _object);
        Handler(ObjectResult&& _object);

        Handler& operator=(std::nullptr_t);
        Handler& operator=(const ObjectResult& _object);
        Handler& operator=(ObjectResult&& _object);

        ObjectResult getObject() const;
        const SourceReference& getSource() const;

        ObjectType& operator*() const requires handler_details::HasObjectAccessV<T>;
        ObjectType* operator->() const requires handler_details::HasObjectAccessV<T>;

        bool isNull() const;
        bool isValid() const;
        explicit operator bool() const;

        void reset();

    private:
        SourceReference m_source = nullptr;
    };

    template <typename TResult, typename TOwner, typename TResolver>
    class ResolvedHandlerSource final : public HandlerSource<TResult>
    {
    public:
        template <typename TOwnerArg, typename TResolverArg>
        ResolvedHandlerSource(TOwnerArg&& _owner, TResolverArg&& _resolver);

        virtual TResult getObject() const override;

    private:
        TOwner m_owner;
        TResolver m_resolver;
    };

    template <typename TResult, typename TOwner, typename TResolver>
    Handler<TResult> MakeHandler(TOwner&& _owner, TResolver&& _resolver);

    template <typename TResult>
    Handler<TResult> MakeHandler(const TResult& _object);
}

#define EGO_HANDLER_DECLARATION(_TYPE, _NAME, _POSTFIX) \
    using _NAME##_POSTFIX = ego::Handler<_TYPE>;

#define EGO_HANDLER(_TYPE) \
    EGO_HANDLER_DECLARATION(_TYPE, _TYPE, Handler)

#define EGO_NAMED_HANDLER(_TYPE, _NAME) \
    EGO_HANDLER_DECLARATION(_TYPE, _NAME, Handler)

#include "Handler.hpp"
