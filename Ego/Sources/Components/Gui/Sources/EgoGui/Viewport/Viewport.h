#pragma once

#include <cstddef>
#include <memory>

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Patterns/NonInstanceable.h"
#include "EgoCore/Reference/Pointer.h"

#include "EgoGraphicHardware/Presentation/GraphicPresenter.h"

#include "EgoGui/Docking/DockingTypes.h"

#include "ViewportTypes.h"

namespace ego::gui
{
    class InputEvent;
    class InputState;
    class PaintContext;
    class SurfaceRoot;
    class ViewportInputContext;
    class ViewportManager;
    class Widget;
    class Window;
    struct LayoutContext;
    struct ViewportUpdate;

    EGO_POINTER(SurfaceRoot);
    EGO_POINTER(Widget);
    EGO_POINTER(Window);

    class Viewport final : public NonCopyable
    {
    public:
        class ViewportManagerAccessor final : public NonInstanceable
        {
            friend class ViewportManager;

            static ego::SharedPointer<Viewport> Create(ViewportID _id, const ViewportDesc& _desc);
            static ViewportID GetID(const Viewport& _viewport);
            static const Position& GetPosition(const Viewport& _viewport);
            static const Size& GetSize(const Viewport& _viewport);
            static GraphicPresenterPointer GetGraphicPresenterPointer(const Viewport& _viewport);
            static void SetPosition(Viewport& _viewport, const Position& _position);
            static void SetSize(Viewport& _viewport, const Size& _size);
            static bool ContainsWindow(const Viewport& _viewport, const WindowPointer& _window);
            static size_t GetWindowCount(const Viewport& _viewport);
            static WindowPointer GetWindow(const Viewport& _viewport, size_t _index);
            static size_t GetFloatingWindowCount(const Viewport& _viewport);
            static WindowPointer GetFloatingWindow(const Viewport& _viewport, size_t _index);
            static bool IsWindowBound(const Viewport& _viewport, const WindowPointer& _window);
            static bool IsWindowFloating(const Viewport& _viewport, const WindowPointer& _window);
            static bool TransferWindowTo(Viewport& _viewport, Viewport& _target, const WindowPointer& _window);
            static void CancelWindowInteraction(Viewport& _viewport, const WindowPointer& _window);
            static void UpdateExternalDragPreview(Viewport& _viewport, const WindowPointer& _window, const Position& _position);
            static bool DockExternalWindow(Viewport& _viewport, const WindowPointer& _window, const Position& _position);
            static bool IsWindowDragActive(const Viewport& _viewport, const WindowPointer& _window);
            static void ClearExternalDragPreview(Viewport& _viewport);
            static void Clear(Viewport& _viewport);
            static void ApplyViewportUpdate(Viewport& _viewport, const ViewportUpdate& _update);
            static bool UpdateLayout(Viewport& _viewport, const LayoutContext& _layoutContext);
            static void ProcessInput(Viewport& _viewport, const InputEvent& _input, ViewportInputContext& _inputContext);
            static void Paint(Viewport& _viewport, PaintContext& _paintContext);
            static void InvalidateLayout(Viewport& _viewport);
        };

        ~Viewport() override;

        bool addWindow(const WindowPointer& _window, bool _isBound = false);
        bool removeWindow(const WindowPointer& _window);
        WidgetPointer getFocusedWidget() const;

        bool setDockingEnabled(bool _isEnabled);
        bool isDockingEnabled() const;
        DockingSpaceID getDefaultDockingSpaceID() const;
        DockingSpaceID getWindowDockingSpaceID(const WindowPointer& _window) const;
        bool moveWindow(const WindowPointer& _window, const DockingLocation& _location);

    private:
        static ego::SharedPointer<Viewport> Create(ViewportID _id, const ViewportDesc& _desc);

        Viewport(ViewportID _id, const ViewportDesc& _desc);

        ViewportID getID() const;
        const Position& getPosition() const;
        const Size& getSize() const;
        GraphicPresenterPointer getGraphicPresenterPointer() const;
        void setPosition(const Position& _position);
        void setSize(const Size& _size);
        bool containsWindow(const WindowPointer& _window) const;
        size_t getWindowCount() const;
        WindowPointer getWindow(size_t _index) const;
        size_t getFloatingWindowCount() const;
        WindowPointer getFloatingWindow(size_t _index) const;
        bool isWindowBound(const WindowPointer& _window) const;
        bool isWindowFloating(const WindowPointer& _window) const;
        bool transferWindowTo(Viewport& _target, const WindowPointer& _window);
        void cancelWindowInteraction(const WindowPointer& _window);
        void updateExternalDragPreview(const WindowPointer& _window, const Position& _position);
        bool dockExternalWindow(const WindowPointer& _window, const Position& _position);
        bool isWindowDragActive(const WindowPointer& _window) const;
        void clearExternalDragPreview();
        void clear();
        void clearInteraction();
        void applyViewportUpdate(const ViewportUpdate& _update);
        bool updateLayout(const LayoutContext& _layoutContext);
        void processInput(const InputEvent& _input, ViewportInputContext& _inputContext);
        void paint(PaintContext& _paintContext);
        void invalidateLayout();
        bool stabilize(const LayoutContext& _layoutContext, InputState& _inputState);
        InputState& getInputState();

        static constexpr size_t MaximumLayoutPassCount = 64;

        ViewportID m_id = InvalidViewportID;
        Position m_position = PositionZero;
        Size m_size = SizeZero;
        GraphicPresenterPointer m_graphicPresenter = nullptr;
        SurfaceRootPointer m_root = nullptr;
        std::unique_ptr<InputState> m_inputState;
    };

    EGO_POINTER(Viewport);
    EGO_WEAK_POINTER(Viewport);
} // namespace ego::gui
