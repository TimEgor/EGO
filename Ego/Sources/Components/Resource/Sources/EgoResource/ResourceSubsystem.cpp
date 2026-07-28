#include "ResourceSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

#include "ResourceController.h"

bool ego::ResourceSubsystem::init(const InitData& _initData)
{
    EGO_CHECK_RETURN_FALSE(_initData.m_resourceFileSystem);

    m_resourceController = MakePointer<ResourceController>();
    EGO_CHECK_RETURN_FALSE(m_resourceController);
    EGO_CHECK_RETURN_FALSE(m_resourceController->init(_initData.m_resourceJobThreadCount, _initData.m_resourceJobThreadName));
    m_resourceController->addFileSystem(_initData.m_resourceFileSystem);

    return true;
}

void ego::ResourceSubsystem::release()
{
    m_resourceProviderPluginController.release();
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_resourceController);
}

ego::ResourceControllerPointer ego::ResourceSubsystem::getResourceControllerPointer() const
{
    return m_resourceController;
}

ego::ResourceController& ego::ResourceSubsystem::getResourceController() const
{
    EGO_ASSERT(m_resourceController);
    return *m_resourceController;
}

ego::ResourceProviderPluginController& ego::ResourceSubsystem::getResourceProviderPluginController()
{
    return m_resourceProviderPluginController;
}

ego::ResourceSubsystemPointer ego::GetResourceSubsystemPointer()
{
    return subsystem::FindSubsystem<ResourceSubsystem>();
}

ego::ResourceSubsystem& ego::GetResourceSubsystem()
{
    const ResourceSubsystemPointer resourceSubsystem = GetResourceSubsystemPointer();
    EGO_ASSERT(resourceSubsystem);

    return *resourceSubsystem;
}
