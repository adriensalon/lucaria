#pragma once

#include <any>

#include <entt/entt.hpp>

#include <lucaria/bin/types_math.hpp>
#include <lucaria/core/utils_detection.hpp>

namespace lucaria {

struct context_game;
struct component_animator;
struct component_interface_screen;
struct component_interface_spatial;
struct component_model_blockout;
struct component_model_unlit;
struct component_rigidbody_passive;
struct component_rigidbody_kinematic;
struct component_rigidbody_dynamic;
struct component_speaker_spatial;
struct component_transform;

namespace detail {

    struct object_scene {
        std::string type_id = {};
        bool is_marked_erase = false;
        std::vector<entt::entity> entities_marked_erase = {};
        entt::registry components = {};
        std::any user_data = {};
    };

    // recipes

    template <typename ComponentType>
    struct recipe_object_scene_component {
        uint32 recipe_component_id = 0;
        ComponentType* component = nullptr;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("recipe_component_id", recipe_component_id));
            archive(cereal::make_nvp("component", *component));
        }
    };

    struct recipe_object_scene_registry {
        std::vector<recipe_object_scene_component<component_animator>> animators = {};
        std::vector<recipe_object_scene_component<component_interface_screen>> screen_interfaces = {};
        std::vector<recipe_object_scene_component<component_interface_spatial>> spatial_interfaces = {};
        std::vector<recipe_object_scene_component<component_model_blockout>> blockout_models = {};
        std::vector<recipe_object_scene_component<component_model_unlit>> unlit_models = {};
        std::vector<recipe_object_scene_component<component_rigidbody_passive>> passive_rigidbodies = {};
        std::vector<recipe_object_scene_component<component_rigidbody_kinematic>> kinematic_rigidbodies = {};
        std::vector<recipe_object_scene_component<component_rigidbody_dynamic>> dynamic_rigidbodies = {};
        std::vector<recipe_object_scene_component<component_speaker_spatial>> speakers = {};
        std::vector<recipe_object_scene_component<component_transform>> transforms = {};

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("animators", animators));
            archive(cereal::make_nvp("screen_interfaces", screen_interfaces));
            archive(cereal::make_nvp("spatial_interfaces", spatial_interfaces));
            archive(cereal::make_nvp("blockout_models", blockout_models));
            archive(cereal::make_nvp("unlit_models", unlit_models));
            archive(cereal::make_nvp("passive_rigidbodies", passive_rigidbodies));
            archive(cereal::make_nvp("kinematic_rigidbodies", kinematic_rigidbodies));
            archive(cereal::make_nvp("dynamic_rigidbodies", dynamic_rigidbodies));
            archive(cereal::make_nvp("speakers", speakers));
            archive(cereal::make_nvp("transforms", transforms));
        }
    };    

}
}
