cmake_minimum_required(VERSION 3.10)

project(EgoDemoStandalone)

add_library(EgoDemoStandalone STATIC)
add_library(Ego::Demo::Standalone ALIAS EgoDemoStandalone)

ego_setup_target_sources(
    EgoDemoStandalone
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Common"
    SOURCE_GROUP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Common"
    INCLUDE_SCOPE PUBLIC
)

target_link_libraries(
    EgoDemoStandalone
        PUBLIC
            Ego::Application
            Ego::Engine
        PRIVATE
            Ego::Core
            Ego::Event
            Ego::Plugin
)

ego_setup_target_common(
    EgoDemoStandalone
    "Demo/Standalone"
)

if (WIN32)
    add_subdirectory(Win32)
endif()
