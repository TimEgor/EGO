#pragma once

#include <type_traits>
#include <unordered_map>

#include "ContextScope.h"
#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/Singleton.h"
#include "EgoCore/Reference/Pointer.h"
#include "GlobalContext.h"

namespace ego::context
{
    class ContextStackInterface
    {
    public:
        ContextStackInterface() = default;
        virtual ~ContextStackInterface() = default;

        virtual void pushScope(const ContextScopePointer& _scope) = 0;
        virtual void popScope(const ContextScopePointer& _scope) = 0;

        virtual void setDefaultScope(const ContextScopePointer& _scope) = 0;
        virtual ContextScopePointer getDefaultScope() const = 0;
        virtual ContextScopePointer getCurrentScope() const = 0;

        virtual bool setGlobalContext(rtti::TypeMetaInfoID _contextTypeID, const GlobalContextPointer& _context) = 0;
        virtual void removeGlobalContext(rtti::TypeMetaInfoID _contextTypeID, const GlobalContextPointer& _context) = 0;
        virtual GlobalContextPointer findGlobalContext(rtti::TypeMetaInfoID _contextTypeID) = 0;

        virtual ContextPointer findContext(rtti::TypeMetaInfoID _contextTypeID) = 0;
    };

    EGO_POINTER(ContextStackInterface);

    class ContextStack final : public ContextStackInterface
    {
    public:
        ContextStack() = default;
        ~ContextStack() override = default;

        void pushScope(const ContextScopePointer& _scope) override;
        void popScope(const ContextScopePointer& _scope) override;

        void setDefaultScope(const ContextScopePointer& _scope) override;
        ContextScopePointer getDefaultScope() const override;
        ContextScopePointer getCurrentScope() const override;

        bool setGlobalContext(rtti::TypeMetaInfoID _contextTypeID, const GlobalContextPointer& _context) override;
        void removeGlobalContext(rtti::TypeMetaInfoID _contextTypeID, const GlobalContextPointer& _context) override;
        GlobalContextPointer findGlobalContext(rtti::TypeMetaInfoID _contextTypeID) override;

        ContextPointer findContext(rtti::TypeMetaInfoID _contextTypeID) override;

    private:
        ContextPointer findScopedContext(rtti::TypeMetaInfoID _contextTypeID) const;

        using GlobalContextCollection = std::unordered_map<rtti::TypeMetaInfoID, GlobalContextPointer>;

        ContextScopePointer m_defaultScope = nullptr;
        GlobalContextCollection m_globalContexts;
    };

    EGO_POINTER(ContextStack);

    class ContextStackCore final : public Singleton<ContextStackCore>
    {
    public:
        ContextStackCore() = default;

        bool init(const ContextStackInterfacePointer& _contextStack);
        void release();

        ContextStackInterfacePointer getStackPointer() const;
        ContextStackInterface& getStack() const;

        void pushScope(const ContextScopePointer& _scope);
        void popScope(const ContextScopePointer& _scope);

        void setDefaultScope(const ContextScopePointer& _scope);
        ContextScopePointer getDefaultScope() const;
        ContextScopePointer getCurrentScope() const;

        bool setGlobalContext(rtti::TypeMetaInfoID _contextTypeID, const GlobalContextPointer& _context);
        void removeGlobalContext(rtti::TypeMetaInfoID _contextTypeID, const GlobalContextPointer& _context);
        GlobalContextPointer findGlobalContext(rtti::TypeMetaInfoID _contextTypeID) const;

        template <typename TContext>
        bool setGlobalContext(const SharedPointer<TContext>& _context)
        {
            static_assert(std::is_base_of_v<GlobalContext, TContext>);
            return setGlobalContext(EGO_RTTI_TYPE_ID(TContext), _context);
        }

        template <typename TContext>
        void removeGlobalContext(const SharedPointer<TContext>& _context)
        {
            static_assert(std::is_base_of_v<GlobalContext, TContext>);
            removeGlobalContext(EGO_RTTI_TYPE_ID(TContext), _context);
        }

        template <typename TContext>
        SharedPointer<TContext> findGlobalContext() const
        {
            static_assert(std::is_base_of_v<GlobalContext, TContext>);

            GlobalContextPointer context = findGlobalContext(EGO_RTTI_TYPE_ID(TContext));
            return context ? StaticPointerCast<TContext>(context) : nullptr;
        }

        ContextPointer findContext(rtti::TypeMetaInfoID _contextTypeID) const;

        template <typename TContext>
        SharedPointer<TContext> findContext() const
        {
            static_assert(std::is_base_of_v<Context, TContext>);
            static_assert(!std::is_base_of_v<GlobalContext, TContext>);

            ContextPointer context = findContext(EGO_RTTI_TYPE_ID(TContext));
            return context ? StaticPointerCast<TContext>(context) : nullptr;
        }

    private:
        ContextStackInterfacePointer m_contextStack = nullptr;
    };

    template <typename TContext>
    SharedPointer<TContext> FindGlobalContext()
    {
        return ContextStackCore::GetInstance().findGlobalContext<TContext>();
    }

    template <typename TContext>
    SharedPointer<TContext> FindCurrentContext()
    {
        return ContextStackCore::GetInstance().findContext<TContext>();
    }

    class ScopedContextScope final : public NonCopyable
    {
    public:
        explicit ScopedContextScope(const ContextScopePointer& _scope);
        ~ScopedContextScope() override;

    private:
        ContextScopePointer m_scope = nullptr;
    };
} // namespace ego::context
