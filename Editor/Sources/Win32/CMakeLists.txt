cmake_minimum_required(VERSION 3.10)

project(EgoEditorWin32)

add_executable(EgoEditorWin32 WIN32)

ego_setup_target_sources(
    EgoEditorWin32
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
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

ego_install_target(
    EgoEditorWin32
    "Editor"
)

ego_setup_launch_args(
    EgoEditorWin32
)
