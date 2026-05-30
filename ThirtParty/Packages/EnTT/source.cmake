ego_third_party_resolve_path("entt/src" ENTT_INCLUDE_DIR)

if (EXISTS "${ENTT_INCLUDE_DIR}/entt/entity/registry.hpp")
    ego_third_party_create_include_target(EnTT "${ENTT_INCLUDE_DIR}")
    set(EGO_THIRD_PARTY_FOUND TRUE)
endif()
