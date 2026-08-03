#pragma once

#include <array>
#include <string>
#include <string_view>

#include "EgoCore/FileName/FileName.h"
#include "EgoCore/Patterns/NonCopyable.h"

#include "GuiModalWindow.h"

namespace ego::editor
{
    class ProjectCreationWindow final
        : public GuiModalWindow, public NonCopyable
    {
    public:
        ProjectCreationWindow() = default;

    private:
        std::string_view getTitle() const override;
        bool draw() override;

        bool drawProjectNameInput();
        void drawProjectPathInput();
        void drawErrorMessage() const;
        bool drawActions(const FileName& _directory, bool _isSubmitted);

        FileName selectProjectPath() const;

        static std::string TrimText(const char* _text);

        std::string getProjectName() const;
        FileName getProjectDirectory() const;
        bool tryCreateProject(const FileName& _directory);

        std::array<char, 128> m_projectName = {};
        FileName m_projectPath;
        std::string m_errorMessage;
    };

    EGO_POINTER(ProjectCreationWindow);
} // namespace ego::editor
