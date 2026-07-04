#include "GraphicHardwareContext.h"

#include "EgoCore/Assert/AssertCore.h"
#include "EgoCore/Context/ContextStack.h"
#include "EgoCore/UtilsMacros.h"

#include "EgoRuntime/Plugin/PluginController.h"

bool ego::gpu::GraphicHardwareContext::init(const InitData& _initData)
{
    release();

    EGO_CHECK_INITIALIZATION(initGraphicDevice(_initData));
    EGO_CHECK_INITIALIZATION(initGraphicCommandQueue());

    return true;
}

void ego::gpu::GraphicHardwareContext::release()
{
    m_graphicCommandQueue = nullptr;
    EGO_SAFE_RESET_POINTER_WITH_RELEASING(m_graphicDevice);
    m_graphicHardwarePlugin = nullptr;
}

ego::GraphicDevicePointer ego::gpu::GraphicHardwareContext::getGraphicDevicePointer() const
{
    return m_graphicDevice;
}

const ego::GraphicDevice& ego::gpu::GraphicHardwareContext::getGraphicDevice() const
{
    EGO_ASSERT(m_graphicDevice);
    return *m_graphicDevice;
}

ego::GraphicDevice& ego::gpu::GraphicHardwareContext::getGraphicDevice()
{
    EGO_ASSERT(m_graphicDevice);
    return *m_graphicDevice;
}

const ego::gpu::CommandQueueReference& ego::gpu::GraphicHardwareContext::getGraphicCommandQueue() const
{
    EGO_ASSERT(m_graphicCommandQueue);
    return m_graphicCommandQueue;
}

bool ego::gpu::GraphicHardwareContext::initGraphicDevice(const InitData& _initData)
{
    m_graphicHardwarePlugin = _initData.m_graphicHardwarePlugin;
    if (!m_graphicHardwarePlugin)
    {
        const PluginControllerPointer pluginController = GetCurrentPluginController();
        EGO_CHECK_RETURN_FALSE(pluginController);

        m_graphicHardwarePlugin = pluginController->loadPlugin<GraphicHardwarePlugin>();
    }

    EGO_CHECK_RETURN_FALSE(m_graphicHardwarePlugin);

    m_graphicDevice = m_graphicHardwarePlugin->createGraphicDevice();

    GraphicDevice::InitParams initParams;
    initParams.m_debugEnable = _initData.m_debugEnable;
    initParams.m_gpuValidation = _initData.m_gpuValidation;

    EGO_CHECK_RETURN_FALSE(m_graphicDevice && m_graphicDevice->init(initParams));
    return true;
}

bool ego::gpu::GraphicHardwareContext::initGraphicCommandQueue()
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

ego::gpu::GraphicHardwareContextPointer ego::gpu::GetGraphicHardwareContextPointer()
{
    return context::FindGlobalContext<GraphicHardwareContext>();
}

ego::gpu::GraphicHardwareContext& ego::gpu::GetGraphicHardwareContext()
{
    const GraphicHardwareContextPointer graphicHardwareContext = GetGraphicHardwareContextPointer();
    EGO_ASSERT(graphicHardwareContext);

    return *graphicHardwareContext;
}

ego::GraphicDevicePointer ego::gpu::GetGraphicDevicePointer()
{
    const GraphicHardwareContextPointer graphicHardwareContext = GetGraphicHardwareContextPointer();
    return graphicHardwareContext ? graphicHardwareContext->getGraphicDevicePointer() : nullptr;
}

ego::GraphicDevice& ego::gpu::GetGraphicDevice()
{
    const GraphicDevicePointer graphicDevice = GetGraphicDevicePointer();
    EGO_ASSERT(graphicDevice);

    return *graphicDevice;
}

const ego::gpu::CommandQueueReference& ego::gpu::GetGraphicCommandQueue()
{
    return GetGraphicHardwareContext().getGraphicCommandQueue();
}
