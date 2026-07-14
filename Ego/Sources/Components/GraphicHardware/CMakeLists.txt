cmake_minimum_required(VERSION 3.10)

project(EgoGraphicHardware)

ego_add_component(
    EgoGraphicHardware
    ALIAS Ego::GraphicHardware
)

target_link_libraries(
    EgoGraphicHardware
        PUBLIC
            Ego::Core
            Ego::Math
            Ego::Plugin
            Ego::Resource
)
