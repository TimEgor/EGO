#pragma once

#include "EgoEngine/Graphic/RenderHardware/GraphicObjects/Texture.h"

namespace ego
{
    class GraphicPresenter
    {
	public:
		GraphicPresenter() = default;
		virtual ~GraphicPresenter() = default;

		virtual gpu::Texture2DPointer getTargetTexture() = 0;

		virtual void present() = 0;
    };

    EGO_POINTER(GraphicPresenter);
}
