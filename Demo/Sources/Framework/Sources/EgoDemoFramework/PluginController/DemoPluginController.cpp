#include "DemoPluginController.h"

#include "EgoPlugin/PluginController.h"

#include "EgoDemoFramework/DemoController.h"

bool ego::demo::DemoPluginController::init()
{
    PluginControllerCore::GetInstance().getPluginController().getBindingBridge().addBinding(&DemoControllerCore::GetInstance().getController());

    return true;
}