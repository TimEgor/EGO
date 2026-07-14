cmake_minimum_required(VERSION 3.10)

project(EgoDemo)

include("${CMAKE_CURRENT_SOURCE_DIR}/../CMake/EgoProject.cmake")

ego_setup_project()

if (NOT TARGET Ego::Engine)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Ego)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Sources/Demos)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Sources/Standalone)
