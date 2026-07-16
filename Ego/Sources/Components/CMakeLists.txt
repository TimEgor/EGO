cmake_minimum_required(VERSION 3.10)

project(EgoComponents)

add_subdirectory(Core)
add_subdirectory(ECS)
add_subdirectory(Event)
add_subdirectory(Job)
add_subdirectory(Plugin)
add_subdirectory(Resource)
add_subdirectory(Input)
add_subdirectory(GraphicHardware)
add_subdirectory(Gui)
add_subdirectory(GuiRender)
add_subdirectory(Engine)
add_subdirectory(Application)

add_library(EgoComponents INTERFACE)
add_library(Ego::Components ALIAS EgoComponents)

target_link_libraries(EgoComponents
    INTERFACE
        Ego::Core
        Ego::ECS
        Ego::Gui
        Ego::GuiRender
        Ego::GraphicHardware
        Ego::Application
        Ego::Engine
        Ego::Event
        Ego::Input
        Ego::Job
        Ego::Plugin
        Ego::Resource
)
