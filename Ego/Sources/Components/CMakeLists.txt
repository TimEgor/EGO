cmake_minimum_required(VERSION 3.10)

project(EgoComponents)

add_subdirectory(Core)
add_subdirectory(ECS)
add_subdirectory(Engine)
add_subdirectory(Framework)
add_subdirectory(Math)
add_subdirectory(Plugin)

add_library(EgoComponents INTERFACE)
add_library(Ego::Components ALIAS EgoComponents)

target_link_libraries(EgoComponents
    INTERFACE
        EgoCore
        EgoECS
        EgoEngine
        EgoFramework
        EgoMath
        EgoPlugin
)
