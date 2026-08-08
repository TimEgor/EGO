#include "Layer.h"

#include "EgoGui/Inspector/PropertyGui.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void ego::demo::Layer::reset()
{
    m_name = "Directional Light";
    m_clickCount = 0;
    m_renderMode = 0;
    m_enableShadows = true;
    m_showMainWindow = true;
    m_showSceneWindow = true;
    m_showInspectorWindow = true;
    m_showPropertyValuesWindow = true;
    m_showDearImGuiDemo = false;
    m_propertyValues = GuiDemoPropertyValues();
}

void ego::demo::Layer::drawGui()
{
    drawMenu();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    drawPropertyValuesWindow();
}

void ego::demo::Layer::drawMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Windows"))
        {
            ImGui::MenuItem("Property Values", nullptr, &m_showPropertyValuesWindow);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void ego::demo::Layer::drawPropertyValuesWindow()
{
    if (!m_showPropertyValuesWindow)
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(720.0f, 760.0f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Property Values", &m_showPropertyValuesWindow))
    {
        if (ImGui::Button("Reset Values"))
        {
            m_propertyValues = GuiDemoPropertyValues();
        }

        ImGui::SameLine();
        ImGui::TextDisabled("All supported PropertyMetaInfo and PropertyValue variants");

        if (ImGui::BeginTable("Property Values Table", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthStretch, 0.4f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.6f);
            ImGui::TableHeadersRow();

            gui::DrawProperties(&m_propertyValues, GuiDemoPropertyValues::GetMetaInfo());
            gui::DrawProperties(&m_readOnlyPropertyValues, GuiDemoReadOnlyPropertyValues::GetMetaInfo());

            ImGui::EndTable();
        }
    }

    ImGui::End();
}
