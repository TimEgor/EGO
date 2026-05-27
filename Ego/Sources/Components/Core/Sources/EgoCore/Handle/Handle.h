#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/Reference/Reference.h"

#include <cstddef>
#include <functional>

namespace ego
{
    template <typename T>
    class HandleSource : public STDDestroyMTCountable
    {
    public:
        virtual T* getObject() const = 0;
    };

    template <typename T>
    class Handle
    {
    public:
        using ObjectType = T;
        using SourceType = HandleSource<T>;
        using SourceReference = Reference<SourceType>;

        Handle() = default;
        Handle(std::nullptr_t);
        explicit Handle(const SourceReference& _source);

        T* getObject() const;
        const SourceReference& getSource() const;

        bool isValid() const;
        explicit operator bool() const;

        void reset();

    private:
        SourceReference m_source = nullptr;
    };

    template <typename TObject, typename TOwner, typename TResolver>
    class ResolvedHandleSource final : public HandleSource<TObject>
    {
    public:
        template <typename TOwnerArg, typename TResolverArg>
        ResolvedHandleSource(TOwnerArg&& _owner, TResolverArg&& _resolver);

        virtual TObject* getObject() const override;

    private:
        TOwner m_owner;
        TResolver m_resolver;
    };

    template <typename TObject, typename TOwner, typename TResolver>
    Handle<TObject> MakeHandle(TOwner&& _owner, TResolver&& _resolver);

    template <typename TObject>
    Handle<TObject> MakeHandle(TObject* _object);

    template <typename TObject>
    Handle<TObject> MakeHandle(const Reference<TObject>& _object);

    template <typename TObject>
    Handle<TObject> MakeHandle(const SharedPointer<TObject>& _object);
}

#define EGO_HANDLE_DECLARATION(_TYPE, _NAME, _POSTFIX) \
    using _NAME##_POSTFIX = ego::Handle<_TYPE>;

#define EGO_HANDLE(_TYPE) \
    EGO_HANDLE_DECLARATION(_TYPE, _TYPE, Handle)

#define EGO_NAMED_HANDLE(_TYPE, _NAME) \
    EGO_HANDLE_DECLARATION(_TYPE, _NAME, Handle)

#include "Handle.hpp"
