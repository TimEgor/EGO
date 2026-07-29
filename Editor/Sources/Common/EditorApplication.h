#pragma once

#include <string>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoECS/Entity.h"

#include "EgoGraphicHardware/Presentation/TextureGraphicPresenter.h"

#include "EgoGui/Gui.h"

#include "EgoEngine/Engine.h"
#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Level/Level.h"

#include "EgoApplication/Application.h"

#include "GuiLayer.h"

namespace ego
{
    class FileSystem;

    EGO_POINTER(FileSystem);
} // namespace ego

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
            FileName m_pluginDirectoryPath;
            FileName m_profilerPluginName;
            FileName m_renderPluginModuleName;
            FileName m_guiRenderPluginModuleName;
            FileName m_graphicHardwarePluginModuleName;
        };

        struct PresentedSession final
        {
            engine::EngineSessionPointer m_engineSession = nullptr;
            PlatformSurfacePointer m_surface = nullptr;
        };

        struct SceneSession final
        {
            engine::EngineSessionPointer m_engineSession = nullptr;
            TextureGraphicPresenterPointer m_graphicPresenter = nullptr;
        };

        bool initApplication(void* _nativeInstanceHandle, const CommandLineOptions& _options, const XmlDocument& _config);
        void releaseApplication();

        bool loadConfig(XmlDocument& _config) const;
        bool readDefaultFont(const XmlDocument& _config, FileName& _path, float& _size) const;
        bool initEditorAssets(const XmlDocument& _config);
        void releaseEditorAssets();

        bool initEngine(const CommandLineOptions& _options, const XmlDocument& _config);
        void releaseEngine();

        bool createPresentedSession(
            const std::string& _name,
            const SurfaceSize& _size,
            engine::EngineSession::InitData& _sessionInitData,
            PresentedSession& _session);
        void releasePresentedSession(PresentedSession& _session);
        bool createSceneSession(const gpu::Texture2DSize& _size, engine::EngineSession::InitData& _sessionInitData, SceneSession& _session);
        void releaseSceneSession(SceneSession& _session);

        bool initScene();
        void releaseScene();
        void drawSceneEditor();

        bool initEditorUi(const FileName& _fontPath, float _fontSize);
        void releaseEditorUi();

        bool runMainLoop();

        static FileName ResolveOption(const FileName& _option, const XmlNode& _configNode, const char* _configName);
        static void ParseCommandLine(int _argCount, char** _argValues, CommandLineOptions& _options);
        static bool IsSurfaceValid(const PlatformSurfacePointer& _surface);

        application::ApplicationPointer m_application = nullptr;
        engine::EnginePointer m_engine = nullptr;

        PresentedSession m_editorSession;
        SceneSession m_sceneSession;

        LevelPointer m_sceneLevel = nullptr;
        ecs::Entity m_sceneCameraEntity;

        FileName m_renderPluginModuleName;
        FileName m_guiRenderPluginModuleName;

        FileSystemPointer m_editorAssetsFileSystem = nullptr;

        GuiLayer m_guiLayer;
        gui::GuiLayerID m_editorLayerID = gui::InvalidGuiLayerID;
    };
} // namespace ego::editor
