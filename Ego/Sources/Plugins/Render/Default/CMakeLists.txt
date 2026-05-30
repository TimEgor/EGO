cmake_minimum_required(VERSION 3.10)

project(EgoDefaultRender)

ego_add_plugin(
    EgoDefaultRender
    ALIAS Ego::Plugins::Render::Default
    CATEGORY Render
)

target_link_libraries(
    EgoDefaultRender
        PUBLIC
            Ego::Engine
)

ego_register_plugin(
    TARGET EgoDefaultRender
    INSTALL_DIR "Plugins/Render"
    MANIFEST
        "RenderPlugin"
        "Default"
)
