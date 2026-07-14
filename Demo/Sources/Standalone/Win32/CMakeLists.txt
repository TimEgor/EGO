cmake_minimum_required(VERSION 3.10)

project(EgoDemoStandaloneWin32)

add_executable(EgoDemoStandaloneWin32 WIN32)

ego_setup_target_sources(
    EgoDemoStandaloneWin32
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(
    EgoDemoStandaloneWin32
        PRIVATE
            Ego::Demo::Standalone
)

if (TARGET EgoTestDemo)
    add_dependencies(EgoDemoStandaloneWin32 EgoTestDemo)
endif()

if (TARGET EgoDebugDrawDemo)
    add_dependencies(EgoDemoStandaloneWin32 EgoDebugDrawDemo)
endif()

if (TARGET EgoGuiDemo)
    add_dependencies(EgoDemoStandaloneWin32 EgoGuiDemo)
endif()

ego_setup_target_common(
    EgoDemoStandaloneWin32
    "Demo/Standalone"
)

ego_install_target(
    EgoDemoStandaloneWin32
    "Demo/Standalone"
)

ego_setup_launch_args(
    EgoDemoStandaloneWin32
)
