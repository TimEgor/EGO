#include "GraphicHardwareSubsystem.h"

#include "EgoCore/Assert/Assert.h"
#include "EgoCore/Subsystem/SubsystemRegistry.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoPlugin/PluginController.h"

ego::gpu::GraphicHardwareSubsystem::~GraphicHardwareSubsystem()
{
    release();
}

bool ego::gpu::GraphicHardwareSubsystem::init(const InitData& _initData)
{
    release();

    EGO_CHECK_INITIALIZATION(initGraphicDevice(_initData));
    EGO_CHECK_INITIALIZATION(initGraphicCommandQueue());

    return true;
}

void ego::gpu::GraphicHardwareSubsystem::onUnregistered()
{
    release();
}

void ego::gpu::GraphicHardwareSubsystem::release()
{
    m_graphicCommandQueue = nullptr;
    m_graphicDevice = nullptr;
    m_graphicHardwarePlugin = nullptr;
}

ego::GraphicDevicePointer ego::gpu::GraphicHardwareSubsystem::getGraphicDevicePointer() const
{
    return m_graphicDevice;
}

const ego::GraphicDevice& ego::gpu::GraphicHardwareSubsystem::getGraphicDevice() const
{
    EGO_ASSERT(m_graphicDevice);
    return *m_graphicDevice;
}

ego::GraphicDevice& ego::gpu::GraphicHardwareSubsystem::getGraphicDevice()
{
    EGO_ASSERT(m_graphicDevice);
    return *m_graphicDevice;
}

const ego::gpu::CommandQueuePointer& ego::gpu::GraphicHardwareSubsystem::getGraphicCommandQueue() const
{
    EGO_ASSERT(m_graphicCommandQueue);
    return m_graphicCommandQueue;
}

bool ego::gpu::GraphicHardwareSubsystem::initGraphicDevice(const InitData& _initData)
{
    const PluginControllerPointer pluginController = GetPluginControllerPointer();
    EGO_CHECK_RETURN_FALSE(pluginController);

    if (_initData.m_pluginModuleName)
    {
        m_graphicHardwarePlugin = pluginController->loadPlugin<GraphicHardwarePlugin>(_initData.m_pluginModuleName);
    }
    else
    {
        m_graphicHardwarePlugin = pluginController->loadPlugin<GraphicHardwarePlugin>();
    }

    EGO_CHECK_RETURN_FALSE(m_graphicHardwarePlugin);

    m_graphicDevice = m_graphicHardwarePlugin->createGraphicDevice();
    EGO_CHECK_RETURN_FALSE(m_graphicDevice);

    return m_graphicDevice->init(_initData.m_graphicDeviceInitData);
}

bool ego::gpu::GraphicHardwareSubsystem::initGraphicCommandQueue()
{
    EGO_ASSERT(m_graphicDevice);
    EGO_CHECK_RETURN_FALSE(m_graphicDevice);

    CommandQueueDesc commandQueueDesc;
    commandQueueDesc.m_type = CommandType::Graphic;
    commandQueueDesc.m_supportsPresentation = true;

    m_graphicCommandQueue = m_graphicDevice->createCommandQueue(commandQueueDesc);
    EGO_CHECK_RETURN_FALSE(m_graphicCommandQueue);

    return true;
}

ego::gpu::GraphicHardwareSubsystemPointer ego::gpu::GetGraphicHardwareSubsystemPointer()
{
    return subsystem::FindSubsystem<GraphicHardwareSubsystem>();
}

ego::gpu::GraphicHardwareSubsystem& ego::gpu::GetGraphicHardwareSubsystem()
{
    const GraphicHardwareSubsystemPointer graphicHardwareSubsystem = GetGraphicHardwareSubsystemPointer();
    EGO_ASSERT(graphicHardwareSubsystem);

    return *graphicHardwareSubsystem;
}

ego::GraphicDevicePointer ego::gpu::GetGraphicDevicePointer()
{
    const GraphicHardwareSubsystemPointer graphicHardwareSubsystem = GetGraphicHardwareSubsystemPointer();
    return graphicHardwareSubsystem ? graphicHardwareSubsystem->getGraphicDevicePointer() : nullptr;
}

ego::GraphicDevice& ego::gpu::GetGraphicDevice()
{
    const GraphicDevicePointer graphicDevice = GetGraphicDevicePointer();
    EGO_ASSERT(graphicDevice);

    return *graphicDevice;
}

const ego::gpu::CommandQueuePointer& ego::gpu::GetGraphicCommandQueue()
{
    return GetGraphicHardwareSubsystem().getGraphicCommandQueue();
}
