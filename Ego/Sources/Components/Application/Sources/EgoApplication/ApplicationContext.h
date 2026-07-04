#pragma once

#include <functional>
#include <optional>

#include "EgoCore/Context/Context.h"

namespace ego::application
{
    class Application;

    class ApplicationContext final : public context::Context
    {
    public:
        struct InitData final
        {
            Application& m_application;
        };

        ApplicationContext() = default;
        ~ApplicationContext() override = default;

        bool init(const InitData& _initData);
        void release();

        Application& getApplication() const;

        EGO_RTTI_VIRTUAL(ApplicationContext, context::Context);

    private:
        std::optional<std::reference_wrapper<Application>> m_application;
    };

    EGO_POINTER(ApplicationContext);
} // namespace ego::application
