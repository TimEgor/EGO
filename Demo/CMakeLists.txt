cmake_minimum_required(VERSION 3.10)

project(EgoDemo)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Configurations.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Target.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/TargetSources.cmake)

ego_setup_global_properties()

if (NOT TARGET EgoEngine)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Ego)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Sources/Demos)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Sources/Launcher)
