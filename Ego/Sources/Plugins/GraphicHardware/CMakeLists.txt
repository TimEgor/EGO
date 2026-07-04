cmake_minimum_required(VERSION 3.10)

project(EgoPluginsGraphicHardware)

add_library(EgoPluginsGraphicHardware INTERFACE)
add_library(Ego::Plugins::GraphicHardware ALIAS EgoPluginsGraphicHardware)

if (WIN32)
    add_subdirectory(D3D12)

    target_link_libraries(
        EgoPluginsGraphicHardware
            INTERFACE
                Ego::Plugins::GraphicHardware::D3D12
    )
endif()
