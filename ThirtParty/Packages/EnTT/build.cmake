ego_get_third_party_build_roots(ENTT_BUILD_ROOTS)

foreach(ENTT_BUILD_ROOT IN LISTS ENTT_BUILD_ROOTS)
    if (EGO_THIRD_PARTY_FOUND)
        break()
    endif()

    find_package(
        EnTT
        CONFIG
        QUIET
        PATHS "${ENTT_BUILD_ROOT}/EnTT"
        NO_DEFAULT_PATH
    )

    if (TARGET EnTT::EnTT)
        ego_third_party_create_link_target(EnTT EnTT::EnTT)
        set(EGO_THIRD_PARTY_FOUND TRUE)
    endif()
endforeach()
