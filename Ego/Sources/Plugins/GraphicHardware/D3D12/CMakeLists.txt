cmake_minimum_required(VERSION 3.10)

project(EgoD3D12GraphicHardware)

ego_add_plugin(
    EgoD3D12GraphicHardware
    ALIAS Ego::Plugins::GraphicHardware::D3D12
    CATEGORY GraphicHardware
)

target_compile_definitions(EgoD3D12GraphicHardware PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

target_link_libraries(
    EgoD3D12GraphicHardware
        PUBLIC
            Ego::GraphicHardware
        PRIVATE
            d3d12
            dxgi
            dxguid
)

ego_register_plugin(
    TARGET EgoD3D12GraphicHardware
    INSTALL_DIR "Plugins/GraphicHardware"
    MANIFEST
        "GraphicHardwarePlugin"
        "D3D12"
)
