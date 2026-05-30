cmake_minimum_required(VERSION 3.10)

project(EgoPluginsRenderHardware)

add_library(EgoPluginsRenderHardware INTERFACE)
add_library(Ego::Plugins::RenderHardware ALIAS EgoPluginsRenderHardware)

if (WIN32)
    add_subdirectory(D3D12)

    target_link_libraries(
        EgoPluginsRenderHardware
            INTERFACE
                Ego::Plugins::RenderHardware::D3D12
    )
endif()
