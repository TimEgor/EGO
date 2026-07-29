cmake_minimum_required(VERSION 3.10)

project(EgoGui)

ego_add_component(
    EgoGui
    ALIAS Ego::Gui
)

ego_require_third_party(DearImGui)

target_link_libraries(
    EgoGui
        PUBLIC
            Ego::Core
            Ego::GraphicHardware
            Ego::Plugin
            Ego::ThirdParty::DearImGui
        PRIVATE
            Ego::Resource
)
