#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoEngine/Engine.h"

#include "EgoApplication/Application.h"

#include "EditorSubsystem.h"

namespace ego::editor
{
    class EditorApplication final : public NonCopyable
    {
    public:
        static constexpr int SuccessExitCode = 0;
        static constexpr int InitializationFailedExitCode = 1;
        static constexpr int RuntimeFailedExitCode = 2;

        EditorApplication() = default;
        ~EditorApplication() override;

        bool init(void* _nativeInstanceHandle, int _argCount, char** _argValues);
        void release();

        int run();

    private:
        struct CommandLineOptions final
        {
            FileName m_graphicHardwarePluginModuleName;
        };

        bool initApplication(void* _nativeInstanceHandle, const CommandLineOptions& _options, const XmlDocument& _config);
        void releaseApplication();

        bool loadConfig(XmlDocument& _config) const;

        bool initEngine();
        void releaseEngine();

        bool initEditorSubsystem(const XmlDocument& _config);
        void releaseEditorSubsystem();

        bool runMainLoop();

        static FileName ResolveOption(const FileName& _option, const XmlNode& _configNode, const char* _configName);
        static void ParseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options);

        application::ApplicationPointer m_application = nullptr;
        engine::EnginePointer m_engine = nullptr;

        EditorSubsystemPointer m_editorSubsystem = nullptr;
    };
} // namespace ego::editor
