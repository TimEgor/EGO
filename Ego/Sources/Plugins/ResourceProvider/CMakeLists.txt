cmake_minimum_required(VERSION 3.10)

project(EgoPluginsResourceProvider)

add_library(EgoPluginsResourceProvider INTERFACE)
add_library(Ego::Plugins::ResourceProvider ALIAS EgoPluginsResourceProvider)

if (WIN32)
    add_subdirectory(DXC)

    target_link_libraries(
        EgoPluginsResourceProvider
            INTERFACE
                Ego::Plugins::ResourceProvider::DXC
    )
endif()
