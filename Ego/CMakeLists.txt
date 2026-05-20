cmake_minimum_required(VERSION 3.10)

project(Ego)

if (NOT DEFINED EGO_ROOT_PATH)
    set(EGO_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
endif()

include(${EGO_ROOT_PATH}/CMake/Common.cmake)
include(${EGO_ROOT_PATH}/CMake/Configurations.cmake)
include(${EGO_ROOT_PATH}/CMake/Target.cmake)
include(${EGO_ROOT_PATH}/CMake/TargetSources.cmake)
include(${EGO_ROOT_PATH}/CMake/ExternalModule.cmake)

ego_setup_global_properties()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Sources/Components)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Sources/Plugins)
