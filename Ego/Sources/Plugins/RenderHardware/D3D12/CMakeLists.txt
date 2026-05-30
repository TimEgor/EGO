cmake_minimum_required(VERSION 3.10)

project(EgoD3D12RenderHardware)

ego_add_plugin(
    EgoD3D12RenderHardware
    ALIAS Ego::Plugins::RenderHardware::D3D12
    CATEGORY RenderHardware
)

target_compile_definitions(EgoD3D12RenderHardware PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

target_link_libraries(
    EgoD3D12RenderHardware
        PUBLIC
            Ego::Engine
        PRIVATE
            d3d12
            dxgi
            dxguid
)

ego_register_plugin(
    TARGET EgoD3D12RenderHardware
    INSTALL_DIR "Plugins/RenderHardware"
    MANIFEST
        "RenderHardwarePlugin"
        "D3D12"
)
