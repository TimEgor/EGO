#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

struct ImGuiContext;

namespace ego::gui
{
    class ImGuiContextScope final : public NonCopyable
    {
    public:
        explicit ImGuiContextScope(ImGuiContext& _context);
        ~ImGuiContextScope() override;

    private:
        ImGuiContext* m_previousContext = nullptr;
    };
} // namespace ego::gui
