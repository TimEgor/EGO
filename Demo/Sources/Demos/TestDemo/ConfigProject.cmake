set(EGO_TEST_DEMO_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(ego_test_demo_config_project)
    set(TPL_TEST_DEMO_ASSETS_PATH "${EGO_TEST_DEMO_DIR}/Assets")
    set(TPL_TEST_DEMO_PLUGINS_PATH "${CMAKE_INSTALL_PREFIX}/Plugins")
    set(TPL_TEST_DEMO_GAME_PLUGINS_PATH "$<TARGET_FILE_DIR:EgoTestDemo>")

    configure_file(
        "${EGO_TEST_DEMO_DIR}/TestDemoProject.xml.tpl"
        "${CMAKE_CURRENT_BINARY_DIR}/TestDemoProject.xml.in"
        @ONLY
    )
    file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/TestDemoProject.xml"
        INPUT "${CMAKE_CURRENT_BINARY_DIR}/TestDemoProject.xml.in"
    )

    add_custom_command(
        TARGET EgoTestDemo
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/TestDemoProject.xml"
            $<TARGET_FILE_DIR:EgoTestDemo>/TestDemoProject.xml
    )

    if (POLICY CMP0087)
        cmake_policy(SET CMP0087 NEW)
    endif()

    install(CODE
        "set(TPL_TEST_DEMO_ASSETS_PATH \"\${CMAKE_INSTALL_PREFIX}/Demo/Demos/TestDemo/Assets\")
        set(TPL_TEST_DEMO_PLUGINS_PATH \"\${CMAKE_INSTALL_PREFIX}/Plugins\")
        set(TPL_TEST_DEMO_GAME_PLUGINS_PATH \"\${CMAKE_INSTALL_PREFIX}/Demo/Demos/TestDemo\")
        configure_file(
            \"${EGO_TEST_DEMO_DIR}/TestDemoProject.xml.tpl\"
            \"\${CMAKE_INSTALL_PREFIX}/Demo/Demos/TestDemo/TestDemoProject.xml\"
            @ONLY
        )"
    )

    install(DIRECTORY "${EGO_TEST_DEMO_DIR}/Assets" DESTINATION "Demo/Demos/TestDemo" OPTIONAL)
endfunction()
