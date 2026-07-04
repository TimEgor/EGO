include_guard()

include(${CMAKE_CURRENT_LIST_DIR}/PluginManifest.cmake)

function(ego_setup_target_common TARGET_NAME FILTER)
    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME "${TARGET_NAME}_$<CONFIG>_$<PLATFORM_ID>"
        FOLDER "${FILTER}"
    )

    if (MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
            /Zc:preprocessor
            /WX
        )
    else()
        target_compile_options(${TARGET_NAME} PRIVATE
            -Werror
        )
    endif()
endfunction()

function(_ego_create_target_alias TARGET_NAME ALIAS_NAME)
    if (NOT "${ALIAS_NAME}" STREQUAL "")
        add_library(${ALIAS_NAME} ALIAS ${TARGET_NAME})
    endif()
endfunction()

function(ego_add_component TARGET_NAME)
    set(ONE_VALUE_ARGS ALIAS)
    cmake_parse_arguments(EGO_COMPONENT "" "${ONE_VALUE_ARGS}" "" ${ARGN})

    if (EGO_COMPONENT_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ego_add_component received unknown arguments: ${EGO_COMPONENT_UNPARSED_ARGUMENTS}")
    endif()

    add_library(${TARGET_NAME} STATIC)
    _ego_create_target_alias(${TARGET_NAME} "${EGO_COMPONENT_ALIAS}")

    ego_setup_target_sources(
        ${TARGET_NAME}
        INCLUDE_SCOPE PUBLIC
    )

    ego_setup_target_common(
        ${TARGET_NAME}
        "Ego/Components"
    )
endfunction()

function(ego_add_tool TARGET_NAME)
    set(ONE_VALUE_ARGS ALIAS)
    cmake_parse_arguments(EGO_TOOL "" "${ONE_VALUE_ARGS}" "" ${ARGN})

    if (EGO_TOOL_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ego_add_tool received unknown arguments: ${EGO_TOOL_UNPARSED_ARGUMENTS}")
    endif()

    add_library(${TARGET_NAME} STATIC)
    _ego_create_target_alias(${TARGET_NAME} "${EGO_TOOL_ALIAS}")

    ego_setup_target_sources(
        ${TARGET_NAME}
        INCLUDE_SCOPE PUBLIC
    )

    ego_setup_target_common(
        ${TARGET_NAME}
        "Ego/Tools"
    )
endfunction()

function(ego_add_plugin TARGET_NAME)
    set(ONE_VALUE_ARGS ALIAS CATEGORY INCLUDE_SCOPE)
    cmake_parse_arguments(EGO_PLUGIN "" "${ONE_VALUE_ARGS}" "" ${ARGN})

    if (EGO_PLUGIN_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ego_add_plugin received unknown arguments: ${EGO_PLUGIN_UNPARSED_ARGUMENTS}")
    endif()

    if (NOT EGO_PLUGIN_CATEGORY)
        message(FATAL_ERROR "ego_add_plugin requires CATEGORY.")
    endif()

    if (NOT EGO_PLUGIN_INCLUDE_SCOPE)
        set(EGO_PLUGIN_INCLUDE_SCOPE PUBLIC)
    endif()

    add_library(${TARGET_NAME} SHARED)
    _ego_create_target_alias(${TARGET_NAME} "${EGO_PLUGIN_ALIAS}")

    ego_setup_target_sources(
        ${TARGET_NAME}
        INCLUDE_SCOPE ${EGO_PLUGIN_INCLUDE_SCOPE}
    )

    ego_setup_target_common(
        ${TARGET_NAME}
        "Ego/Plugins/${EGO_PLUGIN_CATEGORY}"
    )
endfunction()

function(ego_add_demo_plugin TARGET_NAME)
    set(ONE_VALUE_ARGS ALIAS)
    cmake_parse_arguments(EGO_DEMO_PLUGIN "" "${ONE_VALUE_ARGS}" "" ${ARGN})

    if (EGO_DEMO_PLUGIN_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ego_add_demo_plugin received unknown arguments: ${EGO_DEMO_PLUGIN_UNPARSED_ARGUMENTS}")
    endif()

    add_library(${TARGET_NAME} SHARED)
    _ego_create_target_alias(${TARGET_NAME} "${EGO_DEMO_PLUGIN_ALIAS}")

    ego_setup_target_sources(
        ${TARGET_NAME}
        INCLUDE_SCOPE PRIVATE
    )

    ego_setup_target_common(
        ${TARGET_NAME}
        "Demo/Demos"
    )
endfunction()

function(ego_setup_launch_args TARGET_NAME)
    set(ONE_VALUE_ARGS PROJECT_FILE)
    cmake_parse_arguments(EGO_LAUNCH "" "${ONE_VALUE_ARGS}" "" ${ARGN})

    if (EGO_LAUNCH_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ego_setup_launch_args received unknown arguments: ${EGO_LAUNCH_UNPARSED_ARGUMENTS}")
    endif()

    set(DEBUG_ARGS)
    if (DEFINED EGO_PLUGIN_DIRECTORY)
        set(EGO_LAUNCH_PLUGIN_DIRECTORY "${EGO_PLUGIN_DIRECTORY}")
    else()
        set(EGO_LAUNCH_PLUGIN_DIRECTORY "${CMAKE_BINARY_DIR}/Ego/Sources/Plugins")
    endif()

    if (NOT DEFINED EGO_PROFILER_PLUGIN)
        set(EGO_PROFILER_PLUGIN)

        if (WIN32 AND TARGET EgoWinPIXProfiler)
            set(EGO_PROFILER_PLUGIN "$<TARGET_FILE:EgoWinPIXProfiler>")
            add_dependencies(${TARGET_NAME} EgoWinPIXProfiler)
        endif()
    endif()

    if (NOT "${EGO_PROFILER_PLUGIN}" STREQUAL "")
        string(APPEND DEBUG_ARGS " --profiler=\"${EGO_PROFILER_PLUGIN}\"")
    endif()

    if (NOT DEFINED EGO_RENDER_PLUGIN)
        set(EGO_RENDER_PLUGIN "$<TARGET_FILE:EgoDefaultRender>")
        if (TARGET EgoDefaultRender)
            add_dependencies(${TARGET_NAME} EgoDefaultRender)
        endif()
    endif()

    if (NOT "${EGO_RENDER_PLUGIN}" STREQUAL "")
        string(APPEND DEBUG_ARGS " --render=\"${EGO_RENDER_PLUGIN}\"")
    endif()

    if (NOT DEFINED EGO_GRAPHIC_HARDWARE_PLUGIN)
        set(EGO_GRAPHIC_HARDWARE_PLUGIN)

        if (WIN32)
            set(EGO_GRAPHIC_HARDWARE_PLUGIN "$<TARGET_FILE:EgoD3D12GraphicHardware>")
            if (TARGET EgoD3D12GraphicHardware)
                add_dependencies(${TARGET_NAME} EgoD3D12GraphicHardware)
            endif()
        endif()
    endif()

    if (NOT "${EGO_GRAPHIC_HARDWARE_PLUGIN}" STREQUAL "")
        string(APPEND DEBUG_ARGS " --graphicHardware=\"${EGO_GRAPHIC_HARDWARE_PLUGIN}\"")
    endif()

    if (TARGET EgoDXCResourceProvider)
        add_dependencies(${TARGET_NAME} EgoDXCResourceProvider)
    endif()

    if (NOT "${EGO_LAUNCH_PLUGIN_DIRECTORY}" STREQUAL "")
        string(APPEND DEBUG_ARGS " --pluginDirectory=\"${EGO_LAUNCH_PLUGIN_DIRECTORY}\"")
    endif()

    if (EGO_LAUNCH_PROJECT_FILE)
        string(APPEND DEBUG_ARGS " --project=\"${EGO_LAUNCH_PROJECT_FILE}\"")
    endif()

    if (MSVC)
        set_target_properties(${TARGET_NAME} PROPERTIES
            VS_DEBUGGER_COMMAND_ARGUMENTS "${DEBUG_ARGS}"
        )
    endif()
endfunction()

function(ego_install_target TARGET_NAME DIR)
    install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION ${DIR})
endfunction()

function(ego_register_plugin)
    set(ONE_VALUE_ARGS TARGET INSTALL_DIR)
    set(MULTI_VALUE_ARGS MANIFEST)
    cmake_parse_arguments(EGO_REGISTER_PLUGIN "" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    if (EGO_REGISTER_PLUGIN_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ego_register_plugin received unknown arguments: ${EGO_REGISTER_PLUGIN_UNPARSED_ARGUMENTS}")
    endif()

    if (NOT EGO_REGISTER_PLUGIN_TARGET)
        message(FATAL_ERROR "ego_register_plugin requires TARGET.")
    endif()

    if (NOT TARGET ${EGO_REGISTER_PLUGIN_TARGET})
        message(FATAL_ERROR "ego_register_plugin target '${EGO_REGISTER_PLUGIN_TARGET}' does not exist.")
    endif()

    if (NOT EGO_REGISTER_PLUGIN_INSTALL_DIR)
        message(FATAL_ERROR "ego_register_plugin requires INSTALL_DIR.")
    endif()

    if (NOT EGO_REGISTER_PLUGIN_MANIFEST)
        message(FATAL_ERROR "ego_register_plugin requires MANIFEST entries.")
    endif()

    set_target_properties(
        ${EGO_REGISTER_PLUGIN_TARGET}
        PROPERTIES
            EGO_PLUGIN_INSTALL_DIR "${EGO_REGISTER_PLUGIN_INSTALL_DIR}"
            EGO_PLUGIN_MANIFEST "${EGO_REGISTER_PLUGIN_MANIFEST}"
    )

    set_property(GLOBAL APPEND PROPERTY EGO_REGISTERED_PLUGINS ${EGO_REGISTER_PLUGIN_TARGET})

    ego_install_target(
        ${EGO_REGISTER_PLUGIN_TARGET}
        "${EGO_REGISTER_PLUGIN_INSTALL_DIR}"
    )

    ego_setup_plugin_manifest(
        ${EGO_REGISTER_PLUGIN_TARGET}
        "${EGO_REGISTER_PLUGIN_INSTALL_DIR}"
        ${EGO_REGISTER_PLUGIN_MANIFEST}
    )
endfunction()
