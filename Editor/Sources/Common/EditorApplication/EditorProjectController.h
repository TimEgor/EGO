#pragma once

#include <string>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/Presentation/TextureGraphicPresenter.h"

#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Level/Level.h"
#include "EgoEngine/Project/Project.h"

namespace ego::editor
{
    class EditorProjectController final
        : public NonCopyable
    {
    public:
        EditorProjectController() = default;
        ~EditorProjectController() override;

        bool init(const XmlDocument& _config);
        void release();

        bool isProjectLoaded() const;

        const FileName& getProjectDirectory() const;
        engine::EngineSessionPointer getSimulationSessionPointer() const;
        LevelPointer getCurrentLevelPointer() const;

        void createProject();
        bool createProject(const std::string& _name, const FileName& _directory);
        void loadProject();
        bool saveProject() const;
        void unloadProject();

    private:
        struct ProjectContext final
        {
            engine::ProjectPointer m_project = nullptr;
            FileName m_directory;
            engine::EngineSessionPointer m_simulationSession = nullptr;
            TextureGraphicPresenterPointer m_simulationGraphicPresenter = nullptr;
            LevelPointer m_simulationLevel = nullptr;
        };

        bool readConfig(const XmlDocument& _config);
        FileName selectProjectFile() const;
        void showError(const std::string& _message) const;

        bool initProjectContext(const engine::ProjectPointer& _project, const FileName& _directory);
        bool saveProjectContext() const;
        bool initSimulationGraphicPresenter();
        bool initSimulationSession();

        void releaseSimulationLevel();
        void releaseProjectContext();
        void releaseSimulationSession();
        void releaseSimulationGraphicPresenter();

        ProjectContext m_projectContext;

        FileName m_simulationRenderPluginModuleName;
    };
} // namespace ego::editor
