#include "ContextScope.h"

#include "EgoCore/Assert/AssertCore.h"
#include "GlobalContext.h"

void ego::context::ContextScope::addContext(const ContextPointer& _context)
{
    EGO_ASSERT(_context);
    if (!_context)
    {
        return;
    }

    if (rtti::IsObjectBasedOn<GlobalContext>(*_context))
    {
        EGO_ASSERT_FAIL_MESSAGE("Global context can't be stored in context scope.");
        return;
    }

    m_contexts.push_back(_context);
}

void ego::context::ContextScope::removeContext(const ContextPointer& _context)
{
    EGO_ASSERT(_context);
    if (!_context)
    {
        return;
    }

    for (ContextCollection::iterator contextIter = m_contexts.begin(); contextIter != m_contexts.end(); ++contextIter)
    {
        const ContextPointer& context = *contextIter;
        if (context.get() == _context.get())
        {
            m_contexts.erase(contextIter);
            return;
        }
    }

    EGO_ASSERT_FAIL_MESSAGE("Context isn't stored in scope.");
}

void ego::context::ContextScope::clear()
{
    m_contexts.clear();
}

ego::context::ContextPointer ego::context::ContextScope::findContext(rtti::TypeMetaInfoID _contextTypeID) const
{
    if (_contextTypeID == rtti::InvalidTypeMetaInfoID)
    {
        return nullptr;
    }

    for (auto contextIter = m_contexts.rbegin(); contextIter != m_contexts.rend(); ++contextIter)
    {
        const ContextPointer& context = *contextIter;
        if (context && rtti::IsObjectBasedOn(*context, _contextTypeID))
        {
            return context;
        }
    }

    return nullptr;
}

const ego::context::ContextScope::ContextCollection& ego::context::ContextScope::getContexts() const
{
    return m_contexts;
}

bool ego::context::ContextScope::isEmpty() const
{
    return m_contexts.empty();
}
