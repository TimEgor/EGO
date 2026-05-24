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
            dxgi
            dxguid
)

ego_setup_target_common(
    EgoD3D12RenderHardware
    "Plugins/RenderHardware"
)

ego_install_target(
    EgoD3D12RenderHardware
    "Plugins/RenderHardware"
)

ego_setup_plugin_manifest(
    EgoD3D12RenderHardware
    "Plugins/RenderHardware"
    "RenderHardwarePlugin"
    "D3D12"
)
