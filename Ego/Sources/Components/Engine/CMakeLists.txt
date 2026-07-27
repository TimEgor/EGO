cmake_minimum_required(VERSION 3.10)

project(EgoEngine)

ego_add_component(
    EgoEngine
    ALIAS Ego::Engine
)

target_link_libraries(
    EgoEngine
        PUBLIC
            Ego::Core
            Ego::ECS
            Ego::Gui
            Ego::GraphicHardware
            Ego::Job
            Ego::Plugin
            Ego::Resource
            Ego::Application
)
