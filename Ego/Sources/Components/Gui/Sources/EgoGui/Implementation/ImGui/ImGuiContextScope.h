#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

namespace ego::gui
{
    bool InitializeImGuiModuleRuntime();

    class ImGuiContextScope final : public NonCopyable
    {
    public:
        explicit ImGuiContextScope(void* _context);
        ~ImGuiContextScope() override;

        bool isActive() const;

    private:
        void* m_previousContext = nullptr;
        bool m_isActive = false;
    };
} // namespace ego::gui
