#include "PlatformContext.h"

#include "ContextStack.h"
#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Platform/PlatformFactory.h"
#include "EgoCore/UtilsMacros.h"

bool ego::context::PlatformContext::init(const InitData& _initData)
{
    m_platform = CreatePlatform(_initData.m_nativeInstanceHandle);
    EGO_CHECK_INITIALIZATION(m_platform && m_platform->init());

    return true;
}

void ego::context::PlatformContext::release()
{
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_platform);
}

ego::PlatformPointer ego::context::PlatformContext::getPlatformPointer() const
{
    return m_platform;
}

ego::Platform& ego::context::PlatformContext::getPlatform() const
{
    EGO_ASSERT(m_platform);
    return *m_platform;
}

ego::context::PlatformContextPointer ego::context::GetPlatformContextPointer()
{
    return FindGlobalContext<PlatformContext>();
}

ego::context::PlatformContext& ego::context::GetPlatformContext()
{
    const PlatformContextPointer platformContext = GetPlatformContextPointer();
    EGO_ASSERT(platformContext);

    return *platformContext;
}

ego::PlatformPointer ego::context::GetPlatformPointer()
{
    const PlatformContextPointer platformContext = GetPlatformContextPointer();
    return platformContext ? platformContext->getPlatformPointer() : nullptr;
}

ego::Platform& ego::context::GetPlatform()
{
    const PlatformPointer platform = GetPlatformPointer();
    EGO_ASSERT(platform);

    return *platform;
}
