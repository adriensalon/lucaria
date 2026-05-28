#pragma once

#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/object_animation.hpp>
#include <lucaria/core/utils_access.hpp>
#include <lucaria/core/utils_cache.hpp>

namespace lucaria {

/// @brief Represents an animation on the host. Can be created from an animation file or from empty data.
struct handle_animation {
	
    /// @brief Checks if the texture is ready to be used
    /// @return true if the texture is ready, false otherwise
    bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    explicit operator bool() const;

private:
    detail::flag_refcount _refcount = {};
    detail::container_cache<detail::object_animation>* _cached = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
        const detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
        const uint32 _asset_id = _mappings.objects.animations.get(_cached);
        archive(cereal::make_nvp("object_save_id", _asset_id));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
		uint32 _asset_id = 0;
        archive(cereal::make_nvp("object_save_id", _asset_id));
        detail::mappings_manager_game_load& _mappings = cereal::get_user_data<detail::mappings_manager_game_load>(archive);
        detail::container_cache<detail::object_animation>* _cached_cell = _mappings.objects.animations.get(_asset_id);
        if (_cached_cell == nullptr) {
            LUCARIA_DEBUG_ERROR("Failed to resolve animation handle while loading");
            _cached = nullptr;
            _refcount = {};
            return;
        }
        _cached = _cached_cell;
        _refcount = detail::flag_refcount(&_cached->refs);
    }

    friend struct component_animator;
    friend struct context_object;
    friend struct detail::system_motion;
    friend class cereal::access;
};

}
