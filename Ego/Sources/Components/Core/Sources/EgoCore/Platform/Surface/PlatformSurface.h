#pragma once

#include "EgoCore/Pointer/Pointer.h"
#include "EgoCore/RTTI/RTTI.h"

#include "PlatformSurfaceEvents.h"

namespace ego
{
    class PlatformSurface
    {
    public:
        PlatformSurface() = default;
        virtual ~PlatformSurface();

        const PlatformSurfaceEventIDs& getEventIDs() const;

        virtual bool isValid() const = 0;

        virtual void* getNativeHandle() const = 0;
        virtual bool show(bool _activate = true) = 0;
        virtual void hide() = 0;
        virtual bool isShown() const = 0;

        virtual bool minimize() = 0;
        virtual bool maximize() = 0;
        virtual bool restore() = 0;
        virtual PlatformSurfaceWindowState getWindowState() const = 0;

        virtual bool mapFromScreen(const SurfacePoint& _screenPoint, SurfacePoint& _surfacePoint) const = 0;
        virtual bool mapToScreen(const SurfacePoint& _surfacePoint, SurfacePoint& _screenPoint) const = 0;
        virtual bool setPosition(const SurfacePoint& _position) = 0;
        virtual bool getPosition(SurfacePoint& _position) const = 0;
        virtual bool setSize(const SurfaceSize& _size) = 0;
        virtual const SurfaceSize& getSize() const = 0;

        virtual bool setInputTransparent(bool _isTransparent) = 0;
        virtual bool isInputTransparent() const = 0;
        virtual bool capturePointer() = 0;
        virtual bool releasePointer() = 0;
        virtual bool hasPointerCapture() const = 0;

        virtual bool setCaptionArea(const SurfacePoint& _position, const SurfaceSize& _size) = 0;

        EGO_RTTI_VIRTUAL_BASE(PlatformSurface);

    protected:
        bool initEvents();
        void releaseEvents();

        bool notifyCloseRequested();
        void notifyActivation(bool _isActive);
        void notifyPointerCaptureLost();
        void notifySizeChanged(const SurfaceSize& _previousSize);
        void notifyKeyboardInput(const SurfaceKeyboardInput& _input);
        void notifyTextInput(const SurfaceTextInput& _input);

    private:
        PlatformSurfaceEventIDs m_eventIDs;
    };

    EGO_POINTER(PlatformSurface);
} // namespace ego
