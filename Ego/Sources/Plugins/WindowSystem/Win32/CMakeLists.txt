cmake_minimum_required(VERSION 3.10)

project(EgoWin32WindowSystem)

ego_add_plugin(
    EgoWin32WindowSystem
    ALIAS Ego::Plugins::WindowSystem::Win32
    CATEGORY WindowSystem
    INCLUDE_SCOPE PRIVATE
)

target_compile_definitions(EgoWin32WindowSystem PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

target_link_libraries(
    EgoWin32WindowSystem
        PUBLIC
            Ego::Application
)

ego_register_plugin(
    TARGET EgoWin32WindowSystem
    INSTALL_DIR "Plugins/WindowSystem"
    MANIFEST
        "WindowSystemPlugin"
        "Win32"
)
