#pragma once

#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/object_shape.hpp>
#include <lucaria/core/utils_access.hpp>
#include <lucaria/core/utils_cache.hpp>
#include <lucaria/core/utils_refcount.hpp>

namespace lucaria {

/// @brief Represents runtime geometry meant for collision detection on the device
struct handle_shape {

    bool has_value() const;

    explicit operator bool() const;

private:
    detail::flag_refcount _refcount = {};
    detail::assets_cell<detail::object_shape>* _cached = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
		const detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
        const uint32 _asset_id = _mappings.objects.shapes.get(_cached);
        archive(cereal::make_nvp("object_save_id", _asset_id));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
		uint32 _asset_id = 0;
        archive(cereal::make_nvp("object_save_id", _asset_id));
        detail::mappings_manager_game_load& _mappings = cereal::get_user_data<detail::mappings_manager_game_load>(archive);
        detail::assets_cell<detail::object_shape>* _cached_cell = _mappings.objects.shapes.get(_asset_id);
        if (_cached_cell == nullptr) {
            LUCARIA_DEBUG_ERROR("Failed to resolve shape handle while loading");
            _cached = nullptr;
            _refcount = {};
            return;
        }
        _cached = _cached_cell;
        _refcount = detail::flag_refcount(&_cached->refcount_control);
    }

    friend struct context_object;
    friend struct component_rigidbody_passive;
    friend struct component_rigidbody_kinematic;
    friend struct component_rigidbody_dynamic;
    friend struct detail::system_motion;
    friend struct detail::system_dynamics;
    friend class cereal::access;
};

}
