find_package(EnTT CONFIG QUIET)

if (TARGET EnTT::EnTT)
    ego_third_party_create_link_target(EnTT EnTT::EnTT)
    set(EGO_THIRD_PARTY_FOUND TRUE)
endif()
