find_package(PugiXML CONFIG QUIET)

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
