#pragma once

#include "EgoApplication/EngineApplication.h"

namespace ego::demo::launcher
{
    class LauncherApplication final : public application::EngineApplication
    {
    public:
        LauncherApplication() = default;
        ~LauncherApplication() override;

        static constexpr int SuccessExitCode = 0;
        static constexpr int EngineFrameworkInitializationFailedExitCode = 1;
        static constexpr int ProjectLoadingFailedExitCode = 2;
        static constexpr int ApplicationInitializationFailedExitCode = 3;
        static constexpr int WindowInitializationFailedExitCode = 11;

        int run(void* _nativeInstanceHandle, int _argCount, char** _argValues);

    private:
        int runInternal(void* _nativeInstanceHandle, int _argCount, char** _argValues);
        int initRuntimeFromCommandLine(void* _nativeInstanceHandle, int _argCount, char** _argValues);
        int initApplicationFromOptions(void* _nativeInstanceHandle, const CommandLineOptions& _options);
        int initEngineFrameworkFromOptions(const CommandLineOptions& _options);

        WindowDesc createMainWindowDesc() const override;
        FileName selectProjectFile() const override;
        GraphicPresenterPointer getEngineGraphicPresenter() const override;
        void releaseRuntime();
    };

    EGO_POINTER(LauncherApplication);
} // namespace ego::demo::launcher
