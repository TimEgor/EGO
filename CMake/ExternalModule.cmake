set(EGO_THIRD_PARTY_PATH ${EGO_ROOT_PATH}/ThirtParty)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EGO_THIRD_PARTY_PATH}/FindModules)

function(ego_build_third_party_module MODULE_NAME IS_REQUIRED)
    set(MODULE_NEED_INSTALL FALSE)
    set(MODULE_SOURCE_DIR ${MODULE_NAME})

    set(THIRD_PARTY_MODULE_BUILD_ARGS_PATH ${EGO_THIRD_PARTY_PATH}/BuildArgs/${MODULE_NAME}.cmake)
    if (EXISTS "${THIRD_PARTY_MODULE_BUILD_ARGS_PATH}")
        include("${THIRD_PARTY_MODULE_BUILD_ARGS_PATH}")
    endif()

    set(MODULE_PATH ${EGO_THIRD_PARTY_PATH}/${MODULE_SOURCE_DIR})

    if (NOT EXISTS "${MODULE_PATH}")
        if (${IS_REQUIRED})
            message(FATAL_ERROR "Error: Third party module '${MODULE_NAME}' hasn't been found.")
        else()
            return()
        endif()
    endif()

    set(MODULE_BINARY_PATH ${MODULE_PATH}/Build)
    if (EXISTS "${MODULE_BINARY_PATH}")
        file(REMOVE_RECURSE "${MODULE_BINARY_PATH}")
    endif()

    file(MAKE_DIRECTORY "${MODULE_BINARY_PATH}")

    set(GENERATOR_PLATFORM_ARGS)
    if (CMAKE_GENERATOR_PLATFORM)
        list(APPEND GENERATOR_PLATFORM_ARGS -A ${CMAKE_GENERATOR_PLATFORM})
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} ${GENERATOR_PLATFORM_ARGS} "${MODULE_PATH}" ${MODULE_CONFIG_ARGS}
            -DCMAKE_INSTALL_PREFIX=${EGO_THIRD_PARTY_PATH}/Install/${CMAKE_GENERATOR_PLATFORM}/${MODULE_INSTALL_DIR}
        WORKING_DIRECTORY ${MODULE_BINARY_PATH}
        RESULT_VARIABLE CONFIGURE_RESULT
    )

    if (NOT CONFIGURE_RESULT EQUAL 0)
        message(FATAL_ERROR "Error: Failed to configure third party module '${MODULE_NAME}'.")
    endif()

    if (NOT DEFINED PREBUILD_CONFIGURATIONS)
        set(PREBUILD_CONFIGURATIONS "Debug" "Release")
    endif()

    foreach(CONFIGURATION ${PREBUILD_CONFIGURATIONS})
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build . --config ${CONFIGURATION}
            WORKING_DIRECTORY ${MODULE_BINARY_PATH}
            RESULT_VARIABLE BUILD_RESULT
        )

        if (NOT BUILD_RESULT EQUAL 0)
            message(FATAL_ERROR "Error: Failed to build third party module '${MODULE_NAME}' for ${CONFIGURATION}.")
        endif()
    endforeach()

    if (${MODULE_NEED_INSTALL})
        foreach(CONFIGURATION ${PREBUILD_CONFIGURATIONS})
            execute_process(
                COMMAND ${CMAKE_COMMAND} --install . --config ${CONFIGURATION}
                WORKING_DIRECTORY ${MODULE_BINARY_PATH}
                RESULT_VARIABLE INSTALL_RESULT
            )

            if (NOT INSTALL_RESULT EQUAL 0)
                message(FATAL_ERROR "Error: Failed to install third party module '${MODULE_NAME}' for ${CONFIGURATION}.")
            endif()
        endforeach()
    endif()
endfunction()

function(ego_find_third_party_module MODULE_NAME RESULT)
    set(MODULE_FOUND FALSE)

    if (DEFINED ${MODULE_NAME}_FOUND)
        if (${${MODULE_NAME}_FOUND})
            set(MODULE_FOUND TRUE)
        endif()
    endif()

    if (NOT ${MODULE_FOUND})
        find_package(${MODULE_NAME})

        if (DEFINED ${MODULE_NAME}_FOUND)
            if (${${MODULE_NAME}_FOUND})
                set(MODULE_FOUND TRUE)
            endif()
        endif()
    endif()

    set(${RESULT} ${MODULE_FOUND} PARENT_SCOPE)
endfunction()

function(ego_find_or_build_third_party_module MODULE_NAME RESULT)
    set(MODULE_FOUND TRUE)

    ego_find_third_party_module(${MODULE_NAME} MODULE_FOUND)
    if (NOT ${MODULE_FOUND})
        ego_build_third_party_module(${MODULE_NAME} TRUE)
        ego_find_third_party_module(${MODULE_NAME} MODULE_FOUND)
    endif()

    set(${RESULT} ${MODULE_FOUND} PARENT_SCOPE)
endfunction()

function(ego_import_third_party_module MODULE_NAME)
    set(MODULE_FOUND TRUE)
    ego_find_or_build_third_party_module(${MODULE_NAME} MODULE_FOUND)
    if (NOT ${MODULE_FOUND})
        message(FATAL_ERROR "Error: Third party module '${MODULE_NAME}' hasn't been found.")
    endif()
endfunction()
