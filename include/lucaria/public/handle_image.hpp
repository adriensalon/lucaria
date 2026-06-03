#pragma once

#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/object_image.hpp>
#include <lucaria/core/utils_access.hpp>
#include <lucaria/core/utils_cache.hpp>

namespace lucaria {

/// @brief Represents an image in CPU memory. Can be created from an image file or from an empty size.
struct handle_image {

    /// @brief Checks if the image is ready to be used
    /// @return true if the image is ready, false otherwise
    bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    explicit operator bool() const;

    /// @brief Resizes the image to the specified size
    /// @param size the new size of the image
    void resize(const uint32x2 size);

    /// @brief Gets the size of the image
    /// @return the size of the image
    uint32x2 get_size() const;

private:
    detail::flag_refcount _refcount = {};
    detail::assets_cell<detail::object_image>* _cached = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
		const detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
        const uint32 _asset_id = _mappings.objects.images.get(_cached);
        archive(cereal::make_nvp("object_save_id", _asset_id));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
		uint32 _asset_id = 0;
        archive(cereal::make_nvp("object_save_id", _asset_id));
        detail::mappings_manager_game_load& _mappings = cereal::get_user_data<detail::mappings_manager_game_load>(archive);
        detail::assets_cell<detail::object_image>* _cached_cell = _mappings.objects.images.get(_asset_id);
        if (_cached_cell == nullptr) {
            LUCARIA_DEBUG_ERROR("Failed to resolve image handle while loading");
            _cached = nullptr;
            _refcount = {};
            return;
        }
        _cached = _cached_cell;
        _refcount = detail::flag_refcount(&_cached->refcount_control);
    }

    friend struct context_object;
    friend class cereal::access;
};

}
