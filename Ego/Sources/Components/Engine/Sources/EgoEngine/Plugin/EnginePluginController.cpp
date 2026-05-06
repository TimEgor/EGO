#include "EnginePluginController.h"

#include "EgoPlugin/PluginController.h"

#include "EgoEngine/Engine.h"

bool ego::engine::EnginePluginController::init()
{
    PluginControllerCore::GetInstance().getPluginController().getBindingBridge().addBinding(&EngineCore::GetInstance().getEngine());

    return true;
}
