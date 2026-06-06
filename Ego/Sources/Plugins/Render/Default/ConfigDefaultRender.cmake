set(EGO_DEFAULT_RENDER_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(ego_default_render_config_assets)
    set(EGO_DEFAULT_RENDER_ASSETS_DIR "${EGO_DEFAULT_RENDER_DIR}/Assets")
    set(EGO_DEFAULT_RENDER_CONFIG_TEMPLATE "${EGO_DEFAULT_RENDER_DIR}/DefaultRender.xml.tpl")
    set(EGO_DEFAULT_RENDER_RUNTIME_ASSETS_DIR "Assets")

    set(TPL_DEFAULT_RENDER_DEBUG_VERTEX_SHADER "${EGO_DEFAULT_RENDER_RUNTIME_ASSETS_DIR}/Shaders/DefaultRenderDebugVS.hlsl")
    set(TPL_DEFAULT_RENDER_DEBUG_PIXEL_SHADER "${EGO_DEFAULT_RENDER_RUNTIME_ASSETS_DIR}/Shaders/DefaultRenderDebugPS.hlsl")

    configure_file(
        "${EGO_DEFAULT_RENDER_CONFIG_TEMPLATE}"
        "${CMAKE_CURRENT_BINARY_DIR}/DefaultRender.xml.in"
        @ONLY
    )
    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/DefaultRender.xml"
        INPUT "${CMAKE_CURRENT_BINARY_DIR}/DefaultRender.xml.in"
    )

    add_custom_command(
        TARGET EgoDefaultRender
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:EgoDefaultRender>/${EGO_DEFAULT_RENDER_RUNTIME_ASSETS_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/DefaultRender.xml"
            "$<TARGET_FILE_DIR:EgoDefaultRender>/DefaultRender.xml"
        COMMAND ${CMAKE_COMMAND} -E remove -f
            "$<TARGET_FILE_DIR:EgoDefaultRender>/${EGO_DEFAULT_RENDER_RUNTIME_ASSETS_DIR}/DefaultRender.xml"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${EGO_DEFAULT_RENDER_ASSETS_DIR}/Shaders"
            "$<TARGET_FILE_DIR:EgoDefaultRender>/${EGO_DEFAULT_RENDER_RUNTIME_ASSETS_DIR}/Shaders"
    )

    install(
        DIRECTORY "${EGO_DEFAULT_RENDER_ASSETS_DIR}/Shaders"
        DESTINATION "Plugins/Render/${EGO_DEFAULT_RENDER_RUNTIME_ASSETS_DIR}"
        OPTIONAL
    )

    install(CODE
        "file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/Plugins/Render\")
        file(REMOVE \"\${CMAKE_INSTALL_PREFIX}/Plugins/Render/${EGO_DEFAULT_RENDER_RUNTIME_ASSETS_DIR}/DefaultRender.xml\")
        set(TPL_DEFAULT_RENDER_DEBUG_VERTEX_SHADER \"${EGO_DEFAULT_RENDER_RUNTIME_ASSETS_DIR}/Shaders/DefaultRenderDebugVS.hlsl\")
        set(TPL_DEFAULT_RENDER_DEBUG_PIXEL_SHADER \"${EGO_DEFAULT_RENDER_RUNTIME_ASSETS_DIR}/Shaders/DefaultRenderDebugPS.hlsl\")
        configure_file(
            \"${EGO_DEFAULT_RENDER_CONFIG_TEMPLATE}\"
            \"\${CMAKE_INSTALL_PREFIX}/Plugins/Render/DefaultRender.xml\"
            @ONLY
        )"
    )
endfunction()
