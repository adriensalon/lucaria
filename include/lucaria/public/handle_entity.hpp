#pragma once

#include <entt/entt.hpp>

#include <lucaria/core/manager_scene.hpp>
#include <lucaria/core/utils_access.hpp>

namespace lucaria {

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

/// @brief
struct handle_entity {

    /// @brief
    [[nodiscard]] bool has_value() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_animator& get_animator() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_interface_screen& get_screen_interface() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_interface_spatial& get_spatial_interface() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_model_blockout& get_blockout_model() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_model_unlit& get_unlit_model() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_rigidbody_passive& get_passive_rigidbody() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_rigidbody_kinematic& get_kinematic_rigidbody() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_rigidbody_dynamic& get_dynamic_rigidbody() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_speaker_spatial& get_speaker() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_transform& get_transform() const;

private:
    entt::entity _entity = entt::null;
    entt::registry* _registry = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
        const detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
        const uint32 _scene_id = _mappings.scenes.save_map_scene_ids.at(_registry);
        const uint32 _entity_id = _mappings.scenes.save_map_scene_entities.at(_registry).at(_entity);
        archive(cereal::make_nvp("scene_save_id", _scene_id));
        archive(cereal::make_nvp("entity_save_id", _entity_id));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
        // TODO
    }

    friend struct component_transform;
    friend struct context_object;
    friend struct context_scene;
    friend struct detail::manager_scene;
    friend class cereal::access;
};

}