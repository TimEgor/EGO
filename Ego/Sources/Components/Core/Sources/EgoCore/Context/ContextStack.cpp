#include "ContextStack.h"

#include <vector>

#include "EgoCore/Assert/AssertCore.h"

namespace
{
    using ScopeStack = std::vector<ego::context::ContextScopePointer>;

    thread_local ScopeStack CurrentContextScopes;
} // namespace

void ego::context::ContextStack::pushScope(const ContextScopePointer& _scope)
{
    EGO_ASSERT(_scope);
    if (!_scope)
    {
        return;
    }

    CurrentContextScopes.push_back(_scope);
}

void ego::context::ContextStack::popScope(const ContextScopePointer& _scope)
{
    EGO_ASSERT(_scope);
    if (!_scope)
    {
        return;
    }

    if (CurrentContextScopes.empty())
    {
        EGO_ASSERT_FAIL_MESSAGE("Context stack is empty.");
        return;
    }

    const ContextScopePointer& currentScope = CurrentContextScopes.back();
    if (_scope && currentScope.get() != _scope.get())
    {
        EGO_ASSERT_FAIL_MESSAGE("Context stack pop order is invalid.");
        return;
    }

    CurrentContextScopes.pop_back();
}

void ego::context::ContextStack::setDefaultScope(const ContextScopePointer& _scope)
{
    m_defaultScope = _scope;
}

ego::context::ContextScopePointer ego::context::ContextStack::getDefaultScope() const
{
    return m_defaultScope;
}

ego::context::ContextScopePointer ego::context::ContextStack::getCurrentScope() const
{
    return CurrentContextScopes.empty() ? m_defaultScope : CurrentContextScopes.back();
}

bool ego::context::ContextStack::setGlobalContext(rtti::TypeMetaInfoID _contextTypeID, const GlobalContextPointer& _context)
{
    EGO_ASSERT(_contextTypeID != rtti::InvalidTypeMetaInfoID);
    EGO_ASSERT(_context);
    if (_contextTypeID == rtti::InvalidTypeMetaInfoID || !_context)
    {
        return false;
    }

    if (!rtti::IsObjectBasedOn(*_context, _contextTypeID))
    {
        EGO_ASSERT_FAIL_MESSAGE("Global context type doesn't match requested type.");
        return false;
    }

    const auto contextIter = m_globalContexts.find(_contextTypeID);
    if (contextIter == m_globalContexts.end())
    {
        m_globalContexts.emplace(_contextTypeID, _context);
        return true;
    }

    if (contextIter->second.get() == _context.get())
    {
        return true;
    }

    EGO_ASSERT_FAIL_MESSAGE("Global context has been already set.");
    return false;
}

void ego::context::ContextStack::removeGlobalContext(rtti::TypeMetaInfoID _contextTypeID, const GlobalContextPointer& _context)
{
    EGO_ASSERT(_contextTypeID != rtti::InvalidTypeMetaInfoID);
    EGO_ASSERT(_context);
    if (_contextTypeID == rtti::InvalidTypeMetaInfoID || !_context)
    {
        return;
    }

    const auto contextIter = m_globalContexts.find(_contextTypeID);
    if (contextIter == m_globalContexts.end())
    {
        EGO_ASSERT_FAIL_MESSAGE("Global context isn't stored.");
        return;
    }

    if (contextIter->second.get() != _context.get())
    {
        EGO_ASSERT_FAIL_MESSAGE("Global context remove request doesn't match stored context.");
        return;
    }

    m_globalContexts.erase(contextIter);
}

ego::context::GlobalContextPointer ego::context::ContextStack::findGlobalContext(rtti::TypeMetaInfoID _contextTypeID)
{
    if (_contextTypeID == rtti::InvalidTypeMetaInfoID)
    {
        return nullptr;
    }

    const auto contextIter = m_globalContexts.find(_contextTypeID);
    return contextIter != m_globalContexts.end() ? contextIter->second : nullptr;
}

ego::context::ContextPointer ego::context::ContextStack::findContext(rtti::TypeMetaInfoID _contextTypeID)
{
    if (_contextTypeID == rtti::InvalidTypeMetaInfoID)
    {
        return nullptr;
    }

    ContextPointer context = findScopedContext(_contextTypeID);
    if (context)
    {
        return context;
    }

    return CurrentContextScopes.empty() && m_defaultScope ? m_defaultScope->findContext(_contextTypeID) : nullptr;
}

ego::context::ContextPointer ego::context::ContextStack::findScopedContext(rtti::TypeMetaInfoID _contextTypeID) const
{
    for (auto scopeIter = CurrentContextScopes.rbegin(); scopeIter != CurrentContextScopes.rend(); ++scopeIter)
    {
        const ContextScopePointer& scope = *scopeIter;
        if (!scope)
        {
            continue;
        }

        const ContextPointer context = scope->findContext(_contextTypeID);
        if (context)
        {
            return context;
        }
    }

    return nullptr;
}

bool ego::context::ContextStackCore::init(const ContextStackInterfacePointer& _contextStack)
{
    EGO_ASSERT(_contextStack);
    if (!_contextStack)
    {
        return false;
    }

    if (m_contextStack && m_contextStack.get() != _contextStack.get())
    {
        EGO_ASSERT_FAIL_MESSAGE("Context stack core has been already initialized.");
        return false;
    }

    m_contextStack = _contextStack;
    return true;
}

void ego::context::ContextStackCore::release()
{
    m_contextStack = nullptr;
}

ego::context::ContextStackInterfacePointer ego::context::ContextStackCore::getStackPointer() const
{
    return m_contextStack;
}

ego::context::ContextStackInterface& ego::context::ContextStackCore::getStack() const
{
    EGO_ASSERT(m_contextStack);
    return *m_contextStack;
}

void ego::context::ContextStackCore::pushScope(const ContextScopePointer& _scope)
{
    if (!m_contextStack)
    {
        EGO_ASSERT_FAIL_MESSAGE("Context stack core hasn't been initialized.");
        return;
    }

    m_contextStack->pushScope(_scope);
}

void ego::context::ContextStackCore::popScope(const ContextScopePointer& _scope)
{
    if (!m_contextStack)
    {
        EGO_ASSERT_FAIL_MESSAGE("Context stack core hasn't been initialized.");
        return;
    }

    m_contextStack->popScope(_scope);
}

void ego::context::ContextStackCore::setDefaultScope(const ContextScopePointer& _scope)
{
    if (!m_contextStack)
    {
        EGO_ASSERT_FAIL_MESSAGE("Context stack core hasn't been initialized.");
        return;
    }

    m_contextStack->setDefaultScope(_scope);
}

ego::context::ContextScopePointer ego::context::ContextStackCore::getDefaultScope() const
{
    return m_contextStack ? m_contextStack->getDefaultScope() : nullptr;
}

ego::context::ContextScopePointer ego::context::ContextStackCore::getCurrentScope() const
{
    return m_contextStack ? m_contextStack->getCurrentScope() : nullptr;
}

bool ego::context::ContextStackCore::setGlobalContext(rtti::TypeMetaInfoID _contextTypeID, const GlobalContextPointer& _context)
{
    if (!m_contextStack)
    {
        EGO_ASSERT_FAIL_MESSAGE("Context stack core hasn't been initialized.");
        return false;
    }

    return m_contextStack->setGlobalContext(_contextTypeID, _context);
}

void ego::context::ContextStackCore::removeGlobalContext(rtti::TypeMetaInfoID _contextTypeID, const GlobalContextPointer& _context)
{
    if (!m_contextStack)
    {
        EGO_ASSERT_FAIL_MESSAGE("Context stack core hasn't been initialized.");
        return;
    }

    m_contextStack->removeGlobalContext(_contextTypeID, _context);
}

ego::context::GlobalContextPointer ego::context::ContextStackCore::findGlobalContext(rtti::TypeMetaInfoID _contextTypeID) const
{
    if (_contextTypeID == rtti::InvalidTypeMetaInfoID)
    {
        return nullptr;
    }

    return m_contextStack ? m_contextStack->findGlobalContext(_contextTypeID) : nullptr;
}

ego::context::ContextPointer ego::context::ContextStackCore::findContext(rtti::TypeMetaInfoID _contextTypeID) const
{
    if (_contextTypeID == rtti::InvalidTypeMetaInfoID)
    {
        return nullptr;
    }

    return m_contextStack ? m_contextStack->findContext(_contextTypeID) : nullptr;
}

ego::context::ScopedContextScope::ScopedContextScope(const ContextScopePointer& _scope)
    : m_scope(_scope)
{
    if (m_scope)
    {
        ContextStackCore::GetInstance().pushScope(m_scope);
    }
}

ego::context::ScopedContextScope::~ScopedContextScope()
{
    if (m_scope)
    {
        ContextStackCore::GetInstance().popScope(m_scope);
    }
}
