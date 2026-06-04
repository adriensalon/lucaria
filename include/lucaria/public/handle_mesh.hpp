#pragma once

#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/object_mesh.hpp>
#include <lucaria/core/utils_access.hpp>
#include <lucaria/core/utils_cache.hpp>

namespace lucaria {

struct handle_mesh {

    /// @brief Checks if the mesh is ready to be used
    /// @return true if the mesh is ready, false otherwise
    bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    explicit operator bool() const;

// private:
    detail::flag_refcount _refcount = {};
    detail::assets_cell<detail::object_mesh>* _cached = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
        const detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
        const uint32 _asset_id = _mappings.objects.meshes.get(_cached);
        archive(cereal::make_nvp("object_save_id", _asset_id));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
		uint32 _asset_id = 0;
        archive(cereal::make_nvp("object_save_id", _asset_id));
        detail::mappings_manager_game_load& _mappings = cereal::get_user_data<detail::mappings_manager_game_load>(archive);
        detail::assets_cell<detail::object_mesh>* _cached_cell = _mappings.objects.meshes.get(_asset_id);
        if (_cached_cell == nullptr) {
            LUCARIA_DEBUG_ERROR("Failed to resolve mesh handle while loading");
            _cached = nullptr;
            _refcount = {};
            return;
        }
        _cached = _cached_cell;
        _refcount = detail::flag_refcount(&_cached->refcount_control);
    }

    friend struct context_object;
    friend struct detail::system_rendering;
    friend class cereal::access;
};

}
