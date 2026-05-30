if (NOT TARGET pugixml::pugixml)
    set(PUGIXML_BUILD_TESTS OFF CACHE BOOL "Third-party PugiXML option." FORCE)
    set(PUGIXML_INSTALL OFF CACHE BOOL "Third-party PugiXML option." FORCE)
    set(PUGIXML_USE_POSTFIX ON CACHE BOOL "Third-party PugiXML option." FORCE)

    ego_third_party_resolve_path("pugixml" PUGIXML_SOURCE_DIR)
    add_subdirectory(
        "${PUGIXML_SOURCE_DIR}"
        "${CMAKE_BINARY_DIR}/ThirdParty/PugiXML"
        EXCLUDE_FROM_ALL
    )
    ego_third_party_set_targets_folder(
        PugiXML
        pugixml
        pugixml-static
    )
endif()

if (TARGET pugixml::pugixml)
    ego_third_party_create_link_target(PugiXML pugixml::pugixml)
    set(EGO_THIRD_PARTY_FOUND TRUE)
endif()
