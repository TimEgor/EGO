#pragma once

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Texture.h"

namespace ego
{
    class GraphicPresenter
    {
    public:
        GraphicPresenter() = default;
        virtual ~GraphicPresenter() = default;

        virtual void release() = 0;

        virtual gpu::Texture2DReference getTargetTexture() = 0;

        virtual void present() = 0;
    };

    EGO_POINTER(GraphicPresenter);
} // namespace ego
