set(EGO_GUI_DEMO_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(ego_gui_demo_config_project)
    set(TPL_GUI_DEMO_PLUGINS_PATH "${CMAKE_INSTALL_PREFIX}/Plugins")
    set(TPL_GUI_DEMO_GAME_PLUGINS_PATH "$<TARGET_FILE_DIR:EgoGuiDemo>")

    configure_file(
        "${EGO_GUI_DEMO_DIR}/GuiDemoProject.xml.tpl"
        "${CMAKE_CURRENT_BINARY_DIR}/GuiDemoProject.xml.in"
        @ONLY
    )
    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/GuiDemoProject.xml"
        INPUT "${CMAKE_CURRENT_BINARY_DIR}/GuiDemoProject.xml.in"
    )

    add_custom_command(
        TARGET EgoGuiDemo
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/GuiDemoProject.xml"
            $<TARGET_FILE_DIR:EgoGuiDemo>/GuiDemoProject.xml
    )

    if (POLICY CMP0087)
        cmake_policy(SET CMP0087 NEW)
    endif()

    install(CODE
        "set(TPL_GUI_DEMO_PLUGINS_PATH \"\${CMAKE_INSTALL_PREFIX}/Plugins\")
        set(TPL_GUI_DEMO_GAME_PLUGINS_PATH \"\${CMAKE_INSTALL_PREFIX}/Demo/Demos/GuiDemo\")
        configure_file(
            \"${EGO_GUI_DEMO_DIR}/GuiDemoProject.xml.tpl\"
            \"\${CMAKE_INSTALL_PREFIX}/Demo/Demos/GuiDemo/GuiDemoProject.xml\"
            @ONLY
        )"
    )
endfunction()
