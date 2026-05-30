cmake_minimum_required(VERSION 3.10)

project(EgoPluginsPlatform)

add_library(EgoPluginsPlatform INTERFACE)
add_library(Ego::Plugins::Platform ALIAS EgoPluginsPlatform)

if (ANDROID)
    add_subdirectory(Android)
    target_link_libraries(EgoPluginsPlatform INTERFACE EgoAndroidPlatform)
elseif (WIN32)
    add_subdirectory(Win32)
    target_link_libraries(EgoPluginsPlatform INTERFACE Ego::Plugins::Platform::Win32)
endif()
