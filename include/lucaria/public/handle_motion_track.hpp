#pragma once

#include <lucaria/core/manager_scene.hpp>
#include <lucaria/core/object_motion_track.hpp>
#include <lucaria/core/utils_access.hpp>
#include <lucaria/core/utils_cache.hpp>

namespace lucaria {

/// @brief Represents a motion track on the host. Can be created from a motion track file or from empty data.
struct handle_motion_track {

    /// @brief Checks if the texture is ready to be used
    /// @return true if the texture is ready, false otherwise
    bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    explicit operator bool() const;

private:
    detail::flag_refcount _refcount = {};
    detail::container_cache<detail::object_motion_track>* _cached = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
		const detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
        const uint32 _asset_id = _mappings.objects.motion_tracks.get(_cached);
        archive(cereal::make_nvp("object_save_id", _asset_id));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
    }

    friend struct context_object;
    friend struct component_animator;
    friend struct detail::system_motion;
	friend class cereal::access;
};

}
