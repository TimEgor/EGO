cmake_minimum_required(VERSION 3.10)

project(EgoDXCResourceProvider)

ego_add_plugin(
    EgoDXCResourceProvider
    ALIAS Ego::Plugins::ResourceProvider::DXC
    CATEGORY ResourceProvider
)

target_compile_definitions(EgoDXCResourceProvider PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

target_link_libraries(
    EgoDXCResourceProvider
        PUBLIC
            Ego::Core
            Ego::GraphicHardware
        PRIVATE
            dxcompiler
)

if (WIN32)
    if (CMAKE_VS_PLATFORM_NAME STREQUAL "Win32")
        set(DXC_REDIST_ARCH "x86")
    elseif (CMAKE_VS_PLATFORM_NAME STREQUAL "ARM64")
        set(DXC_REDIST_ARCH "arm64")
    else()
        set(DXC_REDIST_ARCH "x64")
    endif()

    unset(EGO_DXCOMPILER_DLL CACHE)
    unset(EGO_DXIL_DLL CACHE)

    set(DXC_RUNTIME_SEARCH_PATHS)
    if (DEFINED CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        list(APPEND DXC_RUNTIME_SEARCH_PATHS
            "$ENV{WindowsSdkDir}/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/${DXC_REDIST_ARCH}"
            "C:/Program Files (x86)/Windows Kits/10/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/${DXC_REDIST_ARCH}"
        )
    endif()

    if (DEFINED CMAKE_SYSTEM_VERSION)
        list(APPEND DXC_RUNTIME_SEARCH_PATHS
            "$ENV{WindowsSdkDir}/bin/${CMAKE_SYSTEM_VERSION}/${DXC_REDIST_ARCH}"
            "C:/Program Files (x86)/Windows Kits/10/bin/${CMAKE_SYSTEM_VERSION}/${DXC_REDIST_ARCH}"
        )
    endif()

    find_file(
        EGO_DXCOMPILER_DLL
        NAMES dxcompiler.dll
        PATHS ${DXC_RUNTIME_SEARCH_PATHS}
        NO_DEFAULT_PATH
    )

    find_file(
        EGO_DXIL_DLL
        NAMES dxil.dll
        PATHS ${DXC_RUNTIME_SEARCH_PATHS}
        NO_DEFAULT_PATH
    )

    if (NOT EGO_DXCOMPILER_DLL OR NOT EGO_DXIL_DLL)
        message(FATAL_ERROR "DXC runtime DLLs were not found for ${DXC_REDIST_ARCH}. Install Windows SDK with Direct3D shader compiler redist.")
    endif()

    add_custom_command(
        TARGET EgoDXCResourceProvider
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${EGO_DXCOMPILER_DLL}"
            "${EGO_DXIL_DLL}"
            "$<TARGET_FILE_DIR:EgoDXCResourceProvider>"
    )

    install(FILES "${EGO_DXCOMPILER_DLL}" "${EGO_DXIL_DLL}" DESTINATION "Plugins/ResourceProvider")
endif()

ego_register_plugin(
    TARGET EgoDXCResourceProvider
    INSTALL_DIR "Plugins/ResourceProvider"
    MANIFEST
        "ResourceProviderPlugin"
        "DXC"
)
