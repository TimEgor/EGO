cmake_minimum_required(VERSION 3.10)

project(EgoD3D12RenderHardware)

add_library(EgoD3D12RenderHardware SHARED)

set(PLUGIN_SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

ego_collect_sources(PLUGIN_SOURCES "${PLUGIN_SOURCES_PATH}")

target_sources(EgoD3D12RenderHardware PRIVATE ${PLUGIN_SOURCES})
source_group(TREE "${PLUGIN_SOURCES_PATH}/EgoD3D12RenderHardware" FILES ${PLUGIN_SOURCES})

target_include_directories(EgoD3D12RenderHardware PUBLIC "${PLUGIN_SOURCES_PATH}")

target_compile_definitions(EgoD3D12RenderHardware PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

target_link_libraries(
    EgoD3D12RenderHardware
        PUBLIC
            EgoEngine
        PRIVATE
            d3d12
            dxcompiler
            dxgi
            dxguid
)

if (WIN32)
    if (CMAKE_VS_PLATFORM_NAME STREQUAL "Win32")
        set(DXC_REDIST_ARCH "x86")
    elseif (CMAKE_VS_PLATFORM_NAME STREQUAL "ARM64")
        set(DXC_REDIST_ARCH "arm64")
    else()
        set(DXC_REDIST_ARCH "x64")
    endif()

    unset(EGO_DXCOMPILER_DLL CACHE)
    unset(EGO_DXIL_DLL CACHE)

    set(DXC_RUNTIME_SEARCH_PATHS)
    if (DEFINED CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        list(APPEND DXC_RUNTIME_SEARCH_PATHS
            "$ENV{WindowsSdkDir}/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/${DXC_REDIST_ARCH}"
            "C:/Program Files (x86)/Windows Kits/10/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/${DXC_REDIST_ARCH}"
        )
    endif()

    if (DEFINED CMAKE_SYSTEM_VERSION)
        list(APPEND DXC_RUNTIME_SEARCH_PATHS
            "$ENV{WindowsSdkDir}/bin/${CMAKE_SYSTEM_VERSION}/${DXC_REDIST_ARCH}"
            "C:/Program Files (x86)/Windows Kits/10/bin/${CMAKE_SYSTEM_VERSION}/${DXC_REDIST_ARCH}"
        )
    endif()

    list(APPEND DXC_RUNTIME_SEARCH_PATHS
        "$ENV{WindowsSdkDir}/Redist/D3D/${DXC_REDIST_ARCH}"
        "C:/Program Files (x86)/Windows Kits/10/Redist/D3D/${DXC_REDIST_ARCH}"
    )

    find_file(
        EGO_DXCOMPILER_DLL
        NAMES dxcompiler.dll
        PATHS ${DXC_RUNTIME_SEARCH_PATHS}
        NO_DEFAULT_PATH
    )

    find_file(
        EGO_DXIL_DLL
        NAMES dxil.dll
        PATHS ${DXC_RUNTIME_SEARCH_PATHS}
        NO_DEFAULT_PATH
    )

    if (NOT EGO_DXCOMPILER_DLL OR NOT EGO_DXIL_DLL)
        message(FATAL_ERROR "DXC runtime DLLs were not found for ${DXC_REDIST_ARCH}. Install Windows SDK with Direct3D shader compiler redist.")
    endif()

    message(STATUS "DXC runtime dxcompiler: ${EGO_DXCOMPILER_DLL}")
    message(STATUS "DXC runtime dxil: ${EGO_DXIL_DLL}")

    add_custom_command(
        TARGET EgoD3D12RenderHardware
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${EGO_DXCOMPILER_DLL}"
            "${EGO_DXIL_DLL}"
            "$<TARGET_FILE_DIR:EgoD3D12RenderHardware>"
    )

    install(FILES "${EGO_DXCOMPILER_DLL}" "${EGO_DXIL_DLL}" DESTINATION "Plugins/RenderHardware")
endif()

ego_setup_target_common(
    EgoD3D12RenderHardware
    "Plugins/RenderHardware"
)

ego_install_target(
    EgoD3D12RenderHardware
    "Plugins/RenderHardware"
)
