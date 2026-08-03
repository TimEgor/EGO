#pragma once

#include "EgoGraphicHardware/GraphicObjects/Texture.h"

namespace ego
{
    class GraphicPresenter
    {
    public:
        GraphicPresenter() = default;
        virtual ~GraphicPresenter() = default;

        virtual bool prepare() = 0;
        virtual gpu::Texture2DPointer getTargetTexture() = 0;
        virtual bool shouldClearTarget() const = 0;
        virtual gpu::GraphicResourceState getPresentationState() const = 0;

        virtual void present() = 0;
    };

    EGO_POINTER(GraphicPresenter);
    EGO_WEAK_POINTER(GraphicPresenter);
} // namespace ego
