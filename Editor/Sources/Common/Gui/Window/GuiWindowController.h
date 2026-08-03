#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "EgoCore/Patterns/NonCopyable.h"

#include "EntityInspectorWindow.h"
#include "GuiModalWindow.h"
#include "GuiWindow.h"
#include "SceneInspectorWindow.h"
#include "ViewportWindow.h"

namespace ego::editor
{
    class GuiWindowController final : public NonCopyable
    {
    public:
        GuiWindowController() = default;

        bool init();
        void release();

        ViewportWindowPointer getViewportWindowPointer() const;
        SceneInspectorWindowPointer getSceneInspectorWindowPointer() const;
        EntityInspectorWindowPointer getEntityInspectorWindowPointer() const;

        bool pushModalWindow(const GuiModalWindowPointer& _window);

        void drawWindows();

    private:
        struct ModalWindowRecord final
        {
            uint64_t m_instanceID = 0;
            GuiModalWindowPointer m_window = nullptr;
        };

        using WindowCollection = std::vector<GuiWindowPointer>;
        using ModalWindowStack = std::vector<ModalWindowRecord>;

        bool initWindows();
        void releaseWindows();
        void drawDockSpace();
        void restoreDefaultDockLayout();
        void drawModalWindow(std::size_t _index);

        WindowCollection m_windows;
        ModalWindowStack m_modalWindowStack;
        uint64_t m_nextModalWindowInstanceID = 0;

        ViewportWindowPointer m_viewportWindow = nullptr;
        SceneInspectorWindowPointer m_sceneInspectorWindow = nullptr;
        EntityInspectorWindowPointer m_entityInspectorWindow = nullptr;
    };
} // namespace ego::editor
