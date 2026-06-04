#pragma once

#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/object_sound_track.hpp>
#include <lucaria/core/utils_access.hpp>
#include <lucaria/core/utils_cache.hpp>

namespace lucaria {

struct handle_sound_track {

    bool has_value() const;

    explicit operator bool() const;

// private:
    detail::flag_refcount _refcount = {};
    detail::assets_cell<detail::object_sound_track>* _cached = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
		const detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
        const uint32 _asset_id = _mappings.objects.sound_tracks.get(_cached);
        archive(cereal::make_nvp("object_save_id", _asset_id));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
		uint32 _asset_id = 0;
        archive(cereal::make_nvp("object_save_id", _asset_id));
        detail::mappings_manager_game_load& _mappings = cereal::get_user_data<detail::mappings_manager_game_load>(archive);
        detail::assets_cell<detail::object_sound_track>* _cached_cell = _mappings.objects.sound_tracks.get(_asset_id);
        if (_cached_cell == nullptr) {
            LUCARIA_DEBUG_ERROR("Failed to resolve sound_track handle while loading");
            _cached = nullptr;
            _refcount = {};
            return;
        }
        _cached = _cached_cell;
        _refcount = detail::flag_refcount(&_cached->refcount_control);
    }

    friend struct context_object;
    friend struct component_speaker_spatial;
    friend class cereal::access;
};

}
