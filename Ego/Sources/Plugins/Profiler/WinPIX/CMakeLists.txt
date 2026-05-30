cmake_minimum_required(VERSION 3.10)

project(EgoWinPIXProfiler)

ego_require_third_party(
    WinPixEventRuntime
    OPTIONAL
    RESULT_VAR WINPIX_PROFILER_MODULE_FOUND
)

if (NOT WINPIX_PROFILER_MODULE_FOUND)
    message(STATUS "WinPixEventRuntime not found. EgoWinPIXProfiler target will not be built.")
    return()
endif()

ego_add_plugin(
    EgoWinPIXProfiler
    ALIAS Ego::Plugins::Profiler::WinPIX
    CATEGORY Profiler
)

target_compile_definitions(EgoWinPIXProfiler PRIVATE USE_PIX)

target_link_libraries(
    EgoWinPIXProfiler
        PUBLIC
            Ego::Engine
        PRIVATE
            Ego::ThirdParty::WinPixEventRuntime
)

ego_copy_third_party_runtime(
    TARGET EgoWinPIXProfiler
    PACKAGE WinPixEventRuntime
    INSTALL_DIR "Plugins/Profiler"
)

ego_register_plugin(
    TARGET EgoWinPIXProfiler
    INSTALL_DIR "Plugins/Profiler"
    MANIFEST
        "ProfilerPlugin"
        "WinPIX"
)
