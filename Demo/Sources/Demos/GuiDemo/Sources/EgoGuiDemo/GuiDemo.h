#pragma once

#include "EgoGui/GuiController.h"

#include "EgoEngine/Level/Level.h"

#include "EgoEngineFramework/EngineLogic.h"

namespace ego::demo
{
    class GuiDemo final : public engine_framework::EngineLogic
    {
    public:
        GuiDemo() = default;

        bool init() override;
        void update(float _deltaTime) override;
        void release() override;

    private:
        bool createGuiTree();
        bool createLevel();

        LevelPointer m_level = nullptr;
        ecs::Entity m_cameraEntity;
    };

    EGO_POINTER(GuiDemo);
} // namespace ego::demo
