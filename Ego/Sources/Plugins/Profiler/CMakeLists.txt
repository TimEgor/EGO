cmake_minimum_required(VERSION 3.10)

project(EgoPluginsProfiler)

add_library(EgoPluginsProfiler INTERFACE)
add_library(Ego::Plugins::Profiler ALIAS EgoPluginsProfiler)

if (WIN32)
    add_subdirectory(WinPIX)

    if (TARGET EgoWinPIXProfiler)
        target_link_libraries(
            EgoPluginsProfiler
                INTERFACE
                    Ego::Plugins::Profiler::WinPIX
        )
    endif()
endif()
