#pragma once

#include "EgoCore/Subsystem/Subsystem.h"
#include "EgoCore/FileName/FileName.h"

#include "GraphicDevice.h"
#include "GraphicHardwarePlugin.h"

namespace ego::gpu
{
    class GraphicHardwareSubsystem final : public subsystem::Subsystem
    {
    public:
        struct InitData final
        {
            FileName m_pluginModuleName;
            GraphicDevice::InitData m_graphicDeviceInitData{true, true};
        };

        GraphicHardwareSubsystem() = default;
        ~GraphicHardwareSubsystem() override = default;

        bool init(const InitData& _initData);
        void release() override;

        GraphicDevicePointer getGraphicDevicePointer() const;
        const GraphicDevice& getGraphicDevice() const;
        GraphicDevice& getGraphicDevice();

        const CommandQueuePointer& getGraphicCommandQueue() const;

        EGO_SUBSYSTEM(GraphicHardwareSubsystem, subsystem::Subsystem);

    private:
        bool initGraphicDevice(const InitData& _initData);
        bool initGraphicCommandQueue();

        GraphicHardwarePluginPointer m_graphicHardwarePlugin = nullptr;
        GraphicDevicePointer m_graphicDevice = nullptr;
        CommandQueuePointer m_graphicCommandQueue = nullptr;
    };

    EGO_POINTER(GraphicHardwareSubsystem);

    GraphicHardwareSubsystemPointer GetGraphicHardwareSubsystemPointer();
    GraphicHardwareSubsystem& GetGraphicHardwareSubsystem();

    GraphicDevicePointer GetGraphicDevicePointer();
    GraphicDevice& GetGraphicDevice();
    const CommandQueuePointer& GetGraphicCommandQueue();
} // namespace ego::gpu
