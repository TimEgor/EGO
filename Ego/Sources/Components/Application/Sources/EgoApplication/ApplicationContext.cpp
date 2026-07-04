#include "ApplicationContext.h"

#include "EgoCore/Assert/AssertCore.h"

bool ego::application::ApplicationContext::init(const InitData& _initData)
{
    m_application = _initData.m_application;
    return true;
}

void ego::application::ApplicationContext::release()
{
    m_application.reset();
}

ego::application::Application& ego::application::ApplicationContext::getApplication() const
{
    EGO_ASSERT(m_application);
    return m_application->get();
}
