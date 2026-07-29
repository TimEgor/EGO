#pragma once

#include <cstdint>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Pointer/Pointer.h"

namespace ego::gui
{
    using GuiStyleChangeID = uint64_t;
    inline constexpr GuiStyleChangeID InvalidGuiStyleChangeID = 0;

    class GuiStyle : public NonCopyable
    {
    public:
        GuiStyle() = default;
        virtual ~GuiStyle() = default;

        virtual void apply() = 0;

        GuiStyleChangeID getChangeID() const;

    protected:
        void markDirty();

    private:
        GuiStyleChangeID m_changeID = 1;
    };

    EGO_POINTER(GuiStyle);
} // namespace ego::gui
