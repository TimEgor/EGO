cmake_minimum_required(VERSION 3.10)

project(EgoEditor)

include("${CMAKE_CURRENT_SOURCE_DIR}/../CMake/EgoProject.cmake")

ego_setup_project()

if (NOT TARGET Ego::Engine)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Ego)
endif()

add_library(EgoEditor STATIC)
add_library(Ego::Editor ALIAS EgoEditor)

ego_setup_target_sources(
    EgoEditor
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Sources/Common"
    SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Sources/Common/EgoEditor"
    INCLUDE_SCOPE PUBLIC
)

target_link_libraries(
    EgoEditor
        PUBLIC
            Ego::Application
            Ego::Runtime
            Ego::Engine
        PRIVATE
            Ego::Core
            Ego::Gui
)

ego_setup_target_common(
    EgoEditor
    "Editor"
)

if (WIN32)
    add_subdirectory(Sources/Win32)
endif()
