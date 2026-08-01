#pragma once

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Parsers/XmlParser/XmlDocument.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "EgoGraphicHardware/Presentation/TextureGraphicPresenter.h"

#include "EgoEngine/EngineSession.h"
#include "EgoEngine/Level/Level.h"
#include "EgoEngine/Project/Project.h"

#include "EditorApplication/Gui/Menu/ProjectMenuLayer.h"

namespace ego::editor
{
    class EditorProjectController final : public NonCopyable
    {
    public:
        EditorProjectController() = default;
        ~EditorProjectController() override;

        bool init(const XmlDocument& _config);
        void release();

        bool isProjectLoaded() const;

        engine::EngineSessionPointer getSimulationSessionPointer() const;
        LevelPointer getCurrentLevelPointer() const;

        void loadProject();
        void unloadProject();

    private:
        struct ProjectContext final
        {
            engine::ProjectPointer m_project = nullptr;
            engine::EngineSessionPointer m_simulationSession = nullptr;
            TextureGraphicPresenterPointer m_simulationGraphicPresenter = nullptr;
            LevelPointer m_simulationLevel = nullptr;
        };

        bool readConfig(const XmlDocument& _config);
        FileName selectProjectFile() const;

        bool initProjectContext(const FileName& _projectFileName);
        bool initSimulationGraphicPresenter();
        bool initSimulationSession();

        void releaseSimulationLevel();
        void releaseProjectContext();
        void releaseSimulationSession();
        void releaseSimulationGraphicPresenter();

        ProjectContext m_projectContext;

        ProjectMenuLayerPointer m_menuLayer = nullptr;

        FileName m_simulationRenderPluginModuleName;
    };
} // namespace ego::editor
