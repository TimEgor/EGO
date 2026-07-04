#pragma once

#include <vector>

#include "Context.h"
#include "EgoCore/RTTI/RTTI.h"

namespace ego::context
{
    class ContextScope final
    {
    public:
        using ContextCollection = std::vector<ContextPointer>;

        ContextScope() = default;
        ~ContextScope() = default;

        void addContext(const ContextPointer& _context);
        void removeContext(const ContextPointer& _context);
        void clear();

        ContextPointer findContext(rtti::TypeMetaInfoID _contextTypeID) const;

        const ContextCollection& getContexts() const;
        bool isEmpty() const;

    private:
        ContextCollection m_contexts;
    };

    EGO_POINTER(ContextScope);
} // namespace ego::context
