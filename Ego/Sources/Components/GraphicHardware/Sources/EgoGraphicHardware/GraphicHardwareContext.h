#pragma once

#include "EgoCore/Context/GlobalContext.h"

#include "GraphicDevice.h"
#include "GraphicHardwarePlugin.h"

namespace ego::gpu
{
    class GraphicHardwareContext final : public context::GlobalContext
    {
    public:
        struct InitData final
        {
            GraphicHardwarePluginPointer m_graphicHardwarePlugin = nullptr;
            bool m_debugEnable = true;
            bool m_gpuValidation = true;
        };

        GraphicHardwareContext() = default;
        ~GraphicHardwareContext() override = default;

        bool init(const InitData& _initData);
        void release();

        GraphicDevicePointer getGraphicDevicePointer() const;
        const GraphicDevice& getGraphicDevice() const;
        GraphicDevice& getGraphicDevice();

        const CommandQueueReference& getGraphicCommandQueue() const;

        EGO_RTTI_VIRTUAL(GraphicHardwareContext, context::GlobalContext);

    private:
        bool initGraphicDevice(const InitData& _initData);
        bool initGraphicCommandQueue();

        GraphicHardwarePluginPointer m_graphicHardwarePlugin = nullptr;
        GraphicDevicePointer m_graphicDevice = nullptr;
        CommandQueueReference m_graphicCommandQueue = nullptr;
    };

    EGO_POINTER(GraphicHardwareContext);

    GraphicHardwareContextPointer GetGraphicHardwareContextPointer();
    GraphicHardwareContext& GetGraphicHardwareContext();

    GraphicDevicePointer GetGraphicDevicePointer();
    GraphicDevice& GetGraphicDevice();
    const CommandQueueReference& GetGraphicCommandQueue();
} // namespace ego::gpu
