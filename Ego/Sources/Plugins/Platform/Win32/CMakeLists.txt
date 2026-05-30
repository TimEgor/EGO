cmake_minimum_required(VERSION 3.10)

project(EgoWin32Platform)

ego_add_plugin(
    EgoWin32Platform
    ALIAS Ego::Plugins::Platform::Win32
    CATEGORY Platform
    INCLUDE_SCOPE PRIVATE
)

target_link_libraries(EgoWin32Platform
    PRIVATE
        Ego::Engine
)

ego_register_plugin(
    TARGET EgoWin32Platform
    INSTALL_DIR "Plugins/Platform"
    MANIFEST
        "PlatformPlugin"
        "Win32"
)
