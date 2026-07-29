#pragma once

#include "EgoCore/Patterns/NonCopyable.h"

namespace ego
{
    class PlatformSurface;
} // namespace ego

namespace ego::editor
{
    class EditorTitleBar final : public NonCopyable
    {
    public:
        void draw(PlatformSurface& _surface, bool& _showViewport, bool& _showSceneInspector, bool& _showEntityInspector);

    private:
        enum class SystemButton
        {
            Minimize,
            Maximize,
            Close
        };

        struct TitleBarLayout;
        struct SystemButtonLayout;

        void drawWindowMenu(bool& _showViewport, bool& _showSceneInspector, bool& _showEntityInspector, TitleBarLayout& _layout) const;
        void drawSystemButtons(PlatformSurface& _surface, TitleBarLayout& _layout) const;
        void drawTitle(const TitleBarLayout& _layout) const;
        void updateCaptionArea(PlatformSurface& _surface, const TitleBarLayout& _layout) const;
        void drawIcon(float _titleBarHeight) const;
        bool drawSystemButton(SystemButton _button, float _positionX, const TitleBarLayout& _layout, bool _isWindowMaximized) const;
        static const char* GetSystemButtonID(SystemButton _button);
        static SystemButtonLayout CreateSystemButtonLayout(float _positionX, const TitleBarLayout& _titleBarLayout);
        void drawSystemButtonBackground(SystemButton _button, const SystemButtonLayout& _layout, bool _isHovered, bool _isActive) const;
        void drawSystemButtonIcon(SystemButton _button, const SystemButtonLayout& _layout, bool _isWindowMaximized) const;
    };
} // namespace ego::editor
