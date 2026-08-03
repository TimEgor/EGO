cmake_minimum_required(VERSION 3.10)

project(EgoEditorWin32)

add_executable(EgoEditorWin32 WIN32)

ego_setup_target_sources(
    EgoEditorWin32
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/EgoEditorWin32"
    SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/EgoEditorWin32"
)

target_link_libraries(
    EgoEditorWin32
        PRIVATE
            Ego::Editor
)

ego_setup_target_common(
    EgoEditorWin32
    "Editor"
)

include(${CMAKE_CURRENT_LIST_DIR}/../../ConfigEditor.cmake)
ego_editor_config(EgoEditorWin32)

ego_install_target(
    EgoEditorWin32
    "Editor"
)
