cmake_minimum_required(VERSION 3.10)

project(Ego)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Configurations.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Target.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/TargetSources.cmake)

ego_setup_global_properties()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Sources/Components)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Sources/Plugins)