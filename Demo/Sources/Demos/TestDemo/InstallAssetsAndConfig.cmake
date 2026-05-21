set(EGO_TEST_DEMO_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(ego_test_demo_install_assets_and_config)
    set(TPL_TEST_DEMO_ASSETS_PATH "${EGO_TEST_DEMO_DIR}/Assets")
    configure_file(
        "${EGO_TEST_DEMO_DIR}/TestDemoConfig.xml.tpl"
        "${CMAKE_CURRENT_BINARY_DIR}/TestDemoConfig.xml"
    )

    add_custom_command(
        TARGET EgoTestDemo
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_BINARY_DIR}/TestDemoConfig.xml"
            $<TARGET_FILE_DIR:EgoTestDemo>/TestDemoConfig.xml
    )

    if (POLICY CMP0087)
        cmake_policy(SET CMP0087 NEW)
    endif()

    install(CODE
        "set(TPL_TEST_DEMO_ASSETS_PATH \"\${CMAKE_INSTALL_PREFIX}/Demo/Demos/TestDemo/Assets\")
        configure_file(
            \"${EGO_TEST_DEMO_DIR}/TestDemoConfig.xml.tpl\"
            \"\${CMAKE_INSTALL_PREFIX}/Demo/Demos/TestDemo/TestDemoConfig.xml\"
        )"
    )

    install(DIRECTORY "${EGO_TEST_DEMO_DIR}/Assets" DESTINATION "Demo/Demos/TestDemo" OPTIONAL)
endfunction()
