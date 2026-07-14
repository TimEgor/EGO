#pragma once

#include "EgoCore/Reference/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"
#include "WindowTypes.h"

namespace ego
{
    class Window
    {
    public:
        Window() = default;
        virtual ~Window() = default;

        virtual bool init(const WindowDesc& _desc) = 0;
        virtual void release() = 0;

        virtual bool isValid() const = 0;

        virtual void show() = 0;
        virtual void hide() = 0;
        virtual bool isShown() const = 0;

        virtual void* getNativeHandle() const = 0;
        virtual bool isStable() const = 0;
        virtual bool screenToClient(const WindowPoint& _screenPoint, WindowPoint& _clientPoint) const = 0;

        virtual const WindowSize& getWindowSize() const = 0;
        virtual const WindowSize& getClientAreaSize() const = 0;
        virtual const WindowArea& getCutoutsArea() const = 0;

        EGO_RTTI_VIRTUAL_BASE(Window);
    };

    EGO_POINTER(Window);
} // namespace ego
