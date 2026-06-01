#pragma once

#include <lucaria/core/serialize_mappings.hpp>
#include <lucaria/core/storage_entity.hpp>
#include <lucaria/core/utils_access.hpp>

namespace lucaria {

/// @brief
struct handle_entity {

    /// @brief
    [[nodiscard]] bool has_value() const;

    // private:
    detail::object_entity _entity = {};

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
            _entity = {};
            return;
        }

        detail::mappings_manager_game_load& mappings = cereal::get_user_data<detail::mappings_manager_game_load>(archive);

        auto scene_it = mappings.scenes.load_map_scenes.find(scene_id);
        if (scene_it == mappings.scenes.load_map_scenes.end()) {
            LUCARIA_DEBUG_ERROR("Failed to resolve scene while loading entity handle");
            _entity = {};
            return;
        }

        auto entities_it = mappings.scenes.load_map_scene_entities.find(scene_id);
        if (entities_it == mappings.scenes.load_map_scene_entities.end()) {
            LUCARIA_DEBUG_ERROR("Failed to resolve scene entity map while loading entity handle");
            _entity = {};
            return;
        }

        auto entity_it = entities_it->second.find(entity_id);
        if (entity_it == entities_it->second.end()) {
            LUCARIA_DEBUG_ERROR("Failed to resolve entity while loading entity handle");
            _entity = {};
            return;
        }

        if (detail::entity_scene(entity_it->second) != scene_it->second
            || mappings.loading_scene_manager == nullptr
            || !mappings.loading_scene_manager->registry.valid(entity_it->second)) {
            LUCARIA_DEBUG_ERROR("Failed to resolve valid entity while loading entity handle");
            _entity = {};
            return;
        }

        _entity = entity_it->second;
    }

    friend struct context_object;
    friend struct context_scene;
    friend struct detail::manager_scenes;
    friend class cereal::access;
};

}