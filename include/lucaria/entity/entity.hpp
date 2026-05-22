#pragma once

#include <lucaria/core/database.hpp>

namespace lucaria {

struct animator_component;
struct screen_interface_component;
struct spatial_interface_component;
struct blockout_model_component;
struct unlit_model_component;
struct passive_rigidbody_component;
struct kinematic_rigidbody_component;
struct dynamic_rigidbody_component;
struct speaker_component;
struct transform_component;

/// @brief
struct scene_entity {

    /// @brief
    [[nodiscard]] bool has_value() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] animator_component& get_animator() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] screen_interface_component& get_screen_interface() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] spatial_interface_component& get_spatial_interface() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] blockout_model_component& get_blockout_model() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] unlit_model_component& get_unlit_model() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] passive_rigidbody_component& get_passive_rigidbody() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] kinematic_rigidbody_component& get_kinematic_rigidbody() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] dynamic_rigidbody_component& get_dynamic_rigidbody() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] speaker_component& get_speaker() const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] transform_component& get_transform() const;

private:
    entt::entity _entity = entt::null;
    entt::registry* _registry = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
        detail::scene_database& _database = detail::engine_scene_database();
        const uint32 _scene_id = _database.save_map_scene_ids.at(_registry);
        const uint32 _entity_id = _database.save_map_scene_entities.at(_registry).at(_entity);
        archive(cereal::make_nvp("scene_save_id", _scene_id));
        archive(cereal::make_nvp("entity_save_id", _entity_id));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
        detail::scene_database& _database = detail::engine_scene_database();
        uint32 _scene_id, _entity_id;
        archive(cereal::make_nvp("scene_save_id", _scene_id));
        archive(cereal::make_nvp("entity_save_id", _entity_id));
        // _registry = _database.load_database.get_registry(_scene_id);
        // _entity = _database.load_database.get_entity(_scene_id, _entity_id);
    }

    friend struct scene_context;
    friend struct transform_component;
    friend class cereal::access;
};

}