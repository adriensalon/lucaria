#pragma once

#include <lucaria/core/manager_scene.hpp>
#include <lucaria/core/object_audio.hpp>
#include <lucaria/core/utils_access.hpp>
#include <lucaria/core/utils_cache.hpp>

namespace lucaria {

/// @brief
struct handle_audio {

    bool has_value() const;

    explicit operator bool() const;

private:
    detail::flag_refcount _refcount = {};
    detail::container_cache<detail::object_audio>* _cached = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
        const detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
        const uint32 _asset_id = _mappings.objects.audios.get(_cached);
        archive(cereal::make_nvp("object_save_id", _asset_id));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
    }

    friend struct context_object;
    friend class cereal::access;
};

}
