cmake_minimum_required(VERSION 3.10)

project(EgoWinPIXProfiler)

set(WINPIX_PROFILER_MODULE_FOUND FALSE)
ego_find_third_party_module(WinPixEventRuntime WINPIX_PROFILER_MODULE_FOUND)

if (NOT WINPIX_PROFILER_MODULE_FOUND)
    message(STATUS "WinPixEventRuntime not found. EgoWinPIXProfiler target will not be built.")
    return()
endif()

add_library(EgoWinPIXProfiler SHARED)

set(PLUGIN_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(PLUGIN_SOURCES "${PLUGIN_SOURCES_PATH}")

target_sources(EgoWinPIXProfiler PRIVATE ${PLUGIN_SOURCES})
source_group(TREE "${PLUGIN_SOURCES_PATH}/EgoWinPIXProfiler" FILES ${PLUGIN_SOURCES})

target_include_directories(EgoWinPIXProfiler PUBLIC "${PLUGIN_SOURCES_PATH}")

target_compile_definitions(EgoWinPIXProfiler PRIVATE USE_PIX)

target_link_libraries(
    EgoWinPIXProfiler
        PUBLIC
            EgoEngine
        PRIVATE
            WinPixEventRuntime
)

if (WinPixEventRuntime_BINARY)
    add_custom_command(
        TARGET EgoWinPIXProfiler
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${WinPixEventRuntime_BINARY}"
            "$<TARGET_FILE_DIR:EgoWinPIXProfiler>/WinPixEventRuntime.dll"
    )

    install(FILES "${WinPixEventRuntime_BINARY}" DESTINATION "Plugins/Profiler" OPTIONAL)
endif()

ego_setup_target_common(
    EgoWinPIXProfiler
    "Plugins/Profiler"
)

ego_install_target(
    EgoWinPIXProfiler
    "Plugins/Profiler"
)

ego_setup_plugin_manifest(
    EgoWinPIXProfiler
    "Plugins/Profiler"
    "ProfilerPlugin"
    "WinPIX"
)
