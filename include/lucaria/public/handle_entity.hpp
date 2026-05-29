#pragma once

#include <lucaria/core/manager_scenes.hpp>
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

    /// @brief Returns true if this entity owns the requested user component.
    /// @tparam ComponentType User component type.
    /// @return true if the entity has the component, false otherwise.
    template <typename ComponentType>
    bool has_component_user() const
    {
        return _manager && _manager->segment_registry_cpu.contains<ComponentType>(_entity);
    }

    /// @brief Gets a user component from this entity.
    /// @tparam ComponentType User component type.
    template <typename ComponentType>
    [[nodiscard]] ComponentType& get_component_user() const
    {
        return _manager->segment_registry_cpu.get<ComponentType>(_entity);
    }

    /// @brief Removes a user component from this entity if present.
    /// @tparam ComponentType User component type.
    template <typename ComponentType>
    void remove_component_user() const
    {
        if (_manager) {
            _manager->segment_registry_cpu.remove<ComponentType>(_entity);
        }
    }

private:
    detail::object_entity _entity = {};
    detail::manager_scenes* _manager = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
        const detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
        const auto scene = detail::entity_scene(_entity);
        const uint32 _scene_id = _mappings.scenes.save_map_scene_ids.at(scene);
        const uint32 _entity_id = _mappings.scenes.save_map_scene_entities.at(scene).at(_entity);
        archive(cereal::make_nvp("scene_save_id", _scene_id));
        archive(cereal::make_nvp("entity_save_id", _entity_id));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
        uint32 scene_id = 0;
        uint32 entity_id = 0;

        archive(cereal::make_nvp("scene_save_id", scene_id));
        archive(cereal::make_nvp("entity_save_id", entity_id));

        if (scene_id == 0 || entity_id == 0) {
            _manager = nullptr;
            _entity = {};
            return;
        }

        detail::mappings_manager_game_load& mappings = cereal::get_user_data<detail::mappings_manager_game_load>(archive);

        auto scene_it = mappings.scenes.load_map_scenes.find(scene_id);
        if (scene_it == mappings.scenes.load_map_scenes.end()) {
            LUCARIA_DEBUG_ERROR("Failed to resolve scene while loading entity handle");
            _manager = nullptr;
            _entity = {};
            return;
        }

        auto entities_it = mappings.scenes.load_map_scene_entities.find(scene_id);
        if (entities_it == mappings.scenes.load_map_scene_entities.end()) {
            LUCARIA_DEBUG_ERROR("Failed to resolve scene entity map while loading entity handle");
            _manager = nullptr;
            _entity = {};
            return;
        }

        auto entity_it = entities_it->second.find(entity_id);
        if (entity_it == entities_it->second.end()) {
            LUCARIA_DEBUG_ERROR("Failed to resolve entity while loading entity handle");
            _manager = nullptr;
            _entity = {};
            return;
        }

        if (detail::entity_scene(entity_it->second) != scene_it->second
            || mappings.loading_scene_manager == nullptr
            || !mappings.loading_scene_manager->segment_registry_cpu.valid(entity_it->second)) {
            LUCARIA_DEBUG_ERROR("Failed to resolve valid entity while loading entity handle");
            _manager = nullptr;
            _entity = {};
            return;
        }

        _manager = mappings.loading_scene_manager;
        _entity = entity_it->second;
    }

    friend struct component_transform;
    friend struct context_object;
    friend struct context_scene;
    friend struct detail::manager_scenes;
    friend class cereal::access;
};

}