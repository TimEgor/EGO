ego_get_third_party_build_roots(PUGIXML_BUILD_ROOTS)

foreach(PUGIXML_BUILD_ROOT IN LISTS PUGIXML_BUILD_ROOTS)
    if (EGO_THIRD_PARTY_FOUND)
        break()
    endif()

    find_package(
        PugiXML
        CONFIG
        QUIET
        PATHS "${PUGIXML_BUILD_ROOT}/PugiXML"
        NO_DEFAULT_PATH
    )

    if (TARGET pugixml::pugixml)
        ego_third_party_apply_imported_config_maps(
            pugixml::static
            DEBUGASAN DEBUG
            RELEASE RELEASE
            RELEASEASAN RELEASE
            PROFILE RELEASE
            PROFILEASAN RELEASE
            RETAIL RELEASE
        )
        ego_third_party_create_link_target(PugiXML pugixml::pugixml)
        set(EGO_THIRD_PARTY_FOUND TRUE)
    endif()
endforeach()
