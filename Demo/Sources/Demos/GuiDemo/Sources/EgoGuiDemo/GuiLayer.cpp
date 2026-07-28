#include "GuiLayer.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void ego::demo::GuiLayer::reset()
{
    m_name = "Directional Light";
    m_clickCount = 0;
    m_renderMode = 0;
    m_enableShadows = true;
    m_showMainWindow = true;
    m_showSceneWindow = true;
    m_showInspectorWindow = true;
    m_showDearImGuiDemo = false;
}

void ego::demo::GuiLayer::drawGui()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Windows"))
        {
            ImGui::MenuItem("Demo Controls", nullptr, &m_showMainWindow);
            ImGui::MenuItem("Scene", nullptr, &m_showSceneWindow);
            ImGui::MenuItem("Inspector", nullptr, &m_showInspectorWindow);
            ImGui::MenuItem("Dear ImGui Demo", nullptr, &m_showDearImGuiDemo);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    if (m_showMainWindow)
    {
        ImGui::SetNextWindowSize(ImVec2(420.0f, 360.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Ego Dear ImGui Demo", &m_showMainWindow))
        {
            const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
            ImGui::Text("Main viewport: %.0f x %.0f", mainViewport->Size.x, mainViewport->Size.y);

            ImGui::SeparatorText("Controls");
            ImGui::InputTextWithHint("Name", "Enter a name", &m_name);
            ImGui::Checkbox("Enable shadows", &m_enableShadows);

            ImGui::TextUnformatted("Render mode");
            ImGui::RadioButton("Shaded", &m_renderMode, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Wireframe", &m_renderMode, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Lighting Only", &m_renderMode, 2);

            if (ImGui::Button("Increment counter"))
            {
                ++m_clickCount;
            }

            ImGui::SameLine();
            ImGui::Text("Clicks: %u", static_cast<unsigned int>(m_clickCount));

            ImGui::SeparatorText("Windows");
            ImGui::Checkbox("Show scene tool", &m_showSceneWindow);
            ImGui::Checkbox("Show inspector tool", &m_showInspectorWindow);
            ImGui::Checkbox("Show Dear ImGui demo", &m_showDearImGuiDemo);
        }

        ImGui::End();
    }

    if (m_showSceneWindow)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 180.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Scene", &m_showSceneWindow))
        {
            ImGui::TextUnformatted("The scene renderer continues to draw the triangle.");
            ImGui::TextWrapped("Dock this tool in the main viewport or move it into a native Dear ImGui viewport.");
        }

        ImGui::End();
    }

    if (m_showInspectorWindow)
    {
        ImGui::SetNextWindowSize(ImVec2(320.0f, 220.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Inspector", &m_showInspectorWindow))
        {
            const char* renderModeName = "Shaded";
            switch (m_renderMode)
            {
            case 1:
                renderModeName = "Wireframe";

                break;
            case 2:
                renderModeName = "Lighting Only";

                break;
            default:
                break;
            }

            ImGui::Text("Name: %s", m_name.c_str());
            ImGui::Text("Shadows: %s", m_enableShadows ? "enabled" : "disabled");
            ImGui::Text("Render mode: %s", renderModeName);
            ImGui::Text("Counter: %u", static_cast<unsigned int>(m_clickCount));
        }

        ImGui::End();
    }

    if (m_showDearImGuiDemo)
    {
        ImGui::ShowDemoWindow(&m_showDearImGuiDemo);
    }
}
