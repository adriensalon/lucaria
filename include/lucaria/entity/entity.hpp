#pragma once

#include <lucaria/core/database.hpp>

namespace lucaria {

/// @brief
struct scene_entity {

    /// @brief
    [[nodiscard]] bool has_value() const;

private:
    entt::entity _entity = entt::null;
    entt::registry* _registry = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
        detail::scene_database& _database = detail::engine_scene_database();
        const uint32 _scene_id = _database.save_database.scene_ids.at(_registry);
        const uint32 _entity_id = _database.save_database.scene_entities.at(_registry).at(_entity);
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
        _registry = _database.load_database.get_registry(_scene_id);
        _entity = _database.load_database.get_entity(_scene_id, _entity_id);
    }

    friend struct scene_context;
    friend struct transform_component;
    friend class cereal::access;
};

}