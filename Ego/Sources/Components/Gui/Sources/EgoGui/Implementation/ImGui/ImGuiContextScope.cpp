#include "ImGuiContextScope.h"

#include <mutex>
#include <thread>

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Memory/Allocator.h"

#include <imgui.h>

namespace
{
    struct ModuleRuntimeState final
    {
        std::mutex m_mutex;
        std::thread::id m_threadID;
        bool m_isInitialized = false;
    };

    ModuleRuntimeState& GetModuleRuntimeState()
    {
        static ModuleRuntimeState State;

        return State;
    }

    void* AllocateImGuiMemory(std::size_t _size, void*)
    {
        return ego::memory::Alloc(_size);
    }

    void FreeImGuiMemory(void* _memory, void*)
    {
        ego::memory::Free(_memory);
    }
} // namespace

bool ego::gui::InitializeImGuiModuleRuntime()
{
    ModuleRuntimeState& state = GetModuleRuntimeState();
    std::lock_guard lock(state.m_mutex);

    if (!state.m_isInitialized)
    {
        ImGui::SetAllocatorFunctions(AllocateImGuiMemory, FreeImGuiMemory);
        if (!IMGUI_CHECKVERSION())
        {
            return false;
        }

        state.m_threadID = std::this_thread::get_id();
        state.m_isInitialized = true;

        return true;
    }

    const bool isRuntimeThread = state.m_threadID == std::this_thread::get_id();
    EGO_ASSERT(isRuntimeThread);

    return isRuntimeThread;
}

ego::gui::ImGuiContextScope::ImGuiContextScope(void* _context)
{
    if (!_context || !InitializeImGuiModuleRuntime())
    {
        return;
    }

    m_previousContext = ImGui::GetCurrentContext();
    ImGui::SetCurrentContext(static_cast<ImGuiContext*>(_context));
    m_isActive = true;
}

ego::gui::ImGuiContextScope::~ImGuiContextScope()
{
    if (!m_isActive)
    {
        return;
    }

    ImGui::SetCurrentContext(static_cast<ImGuiContext*>(m_previousContext));
}

bool ego::gui::ImGuiContextScope::isActive() const
{
    return m_isActive;
}
