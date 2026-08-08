#pragma once

struct ImGuiContext;

namespace ego::gui
{
    class ContextScope final
    {
    public:
        explicit ContextScope(ImGuiContext& _context);
        ~ContextScope();

        ContextScope(const ContextScope&) = delete;
        ContextScope& operator=(const ContextScope&) = delete;

    private:
        ImGuiContext* m_previousContext = nullptr;
    };
} // namespace ego::gui
