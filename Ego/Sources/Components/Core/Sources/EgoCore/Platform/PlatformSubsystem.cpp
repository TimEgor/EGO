#include "PlatformSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/Platform/PlatformFactory.h"
#include "EgoCore/UtilsMacros.h"

ego::PlatformSubsystem::~PlatformSubsystem()
{
    release();
}

bool ego::PlatformSubsystem::init(const InitData& _initData)
{
    m_platform = CreatePlatform(_initData.m_nativeInstanceHandle);
    EGO_CHECK_INITIALIZATION(m_platform && m_platform->init());

    return true;
}

void ego::PlatformSubsystem::onUnregistered()
{
    release();
}

void ego::PlatformSubsystem::release()
{
    EGO_ASSERT(!m_platform || m_platform.getUsingCount() == 1);
    m_platform = nullptr;
}

ego::PlatformPointer ego::PlatformSubsystem::getPlatformPointer() const
{
    return m_platform;
}

ego::Platform& ego::PlatformSubsystem::getPlatform() const
{
    EGO_ASSERT(m_platform);
    return *m_platform;
}

ego::PlatformSubsystemPointer ego::GetPlatformSubsystemPointer()
{
    return subsystem::FindSubsystem<PlatformSubsystem>();
}

ego::PlatformSubsystem& ego::GetPlatformSubsystem()
{
    const PlatformSubsystemPointer platformSubsystem = GetPlatformSubsystemPointer();
    EGO_ASSERT(platformSubsystem);

    return *platformSubsystem;
}

ego::PlatformPointer ego::GetPlatformPointer()
{
    const PlatformSubsystemPointer platformSubsystem = GetPlatformSubsystemPointer();
    return platformSubsystem ? platformSubsystem->getPlatformPointer() : nullptr;
}

ego::Platform& ego::GetPlatform()
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_ASSERT(platform);

    return *platform;
}
