cmake_minimum_required(VERSION 3.10)

project(Ego)

include("${CMAKE_CURRENT_SOURCE_DIR}/../CMake/EgoProject.cmake")

ego_setup_project()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Sources/Components)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Sources/Plugins)
