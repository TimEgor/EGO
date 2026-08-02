set(EGO_DEBUG_DRAW_DEMO_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(ego_debug_draw_demo_config_project)
    set(TPL_DEBUG_DRAW_DEMO_PLUGINS_PATH "${CMAKE_INSTALL_PREFIX}/Plugins")
    set(TPL_DEBUG_DRAW_DEMO_GAME_PLUGINS_PATH "$<TARGET_FILE_DIR:EgoDebugDrawDemo>")

    configure_file(
        "${EGO_DEBUG_DRAW_DEMO_DIR}/DebugDrawDemoProject.egoproj.tpl"
        "${CMAKE_CURRENT_BINARY_DIR}/DebugDrawDemoProject.egoproj.in"
        @ONLY
    )
    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/DebugDrawDemoProject.egoproj"
        INPUT "${CMAKE_CURRENT_BINARY_DIR}/DebugDrawDemoProject.egoproj.in"
    )

    add_custom_command(
        TARGET EgoDebugDrawDemo
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/DebugDrawDemoProject.egoproj"
            $<TARGET_FILE_DIR:EgoDebugDrawDemo>/DebugDrawDemoProject.egoproj
    )

    if (POLICY CMP0087)
        cmake_policy(SET CMP0087 NEW)
    endif()

    install(CODE
        "set(TPL_DEBUG_DRAW_DEMO_PLUGINS_PATH \"\${CMAKE_INSTALL_PREFIX}/Plugins\")
        set(TPL_DEBUG_DRAW_DEMO_GAME_PLUGINS_PATH \"\${CMAKE_INSTALL_PREFIX}/Demo/Demos/DebugDrawDemo\")
        configure_file(
            \"${EGO_DEBUG_DRAW_DEMO_DIR}/DebugDrawDemoProject.egoproj.tpl\"
            \"\${CMAKE_INSTALL_PREFIX}/Demo/Demos/DebugDrawDemo/DebugDrawDemoProject.egoproj\"
            @ONLY
        )"
    )
endfunction()
