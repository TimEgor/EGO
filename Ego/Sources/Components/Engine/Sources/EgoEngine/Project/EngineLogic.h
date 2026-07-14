#pragma once

#include "EgoCore/Patterns/NonCopyable.h"
#include "EgoCore/Reference/Pointer.h"

namespace ego
{
    class ResourceController;
    EGO_POINTER(ResourceController);
} // namespace ego

namespace ego::engine
{
    class EngineSession;

    EGO_POINTER(EngineSession);
    EGO_WEAK_POINTER(EngineSession);

    class EngineLogic : public NonCopyable
    {
    public:
        struct InitData final
        {
            EngineSessionWeakPointer m_engineSession;
            ResourceControllerPointer m_resourceController = nullptr;
        };

        EngineLogic() = default;
        ~EngineLogic() override = default;

        virtual bool init(const InitData& _initData)
        {
            (void)_initData;
            return true;
        }
        virtual void update(float) {}
        virtual void release() {}
    };

    EGO_POINTER(EngineLogic);
    EGO_WEAK_POINTER(EngineLogic);
} // namespace ego::engine
