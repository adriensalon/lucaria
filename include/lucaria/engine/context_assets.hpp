#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/context_window.hpp>

#include <lucaria/engine/asset_animation.hpp>
#include <lucaria/engine/asset_audio.hpp>
#include <lucaria/engine/asset_cubemap.hpp>
#include <lucaria/engine/asset_event_track.hpp>
#include <lucaria/engine/asset_font.hpp>
#include <lucaria/engine/asset_geometry.hpp>
#include <lucaria/engine/asset_image.hpp>
#include <lucaria/engine/asset_mesh.hpp>
#include <lucaria/engine/asset_motion_track.hpp>
#include <lucaria/engine/asset_shape.hpp>
#include <lucaria/engine/asset_skeleton.hpp>
#include <lucaria/engine/asset_sound_track.hpp>
#include <lucaria/engine/asset_texture.hpp>

namespace lucaria {

struct context_window;

/// @brief
struct context_object {

    template <typename AssetType, typename Handle = handle_asset<AssetType>, typename Configure>
    Handle fetch_asset(
        std::string cache_id,
        Configure&& configure,
        context_window* window = nullptr)
    {
        detail::assets_cell<AssetType>& _cell = _manager->assets.template fetch<AssetType>(
            *_manager,
            std::move(cache_id),
            std::forward<Configure>(configure),
            window != nullptr ? window->_manager : nullptr);

        Handle _handle = {};
        _handle._cached = &_cell;
        _handle._refcount = detail::flag_refcount(&_cell.refcount_control);
        return _handle;
    }

    template <typename AssetType, typename Handle = handle_asset<AssetType>, typename... AssetTypeArgs>
    Handle create_asset(AssetTypeArgs&&... args)
    {
        Handle _handle = {};
        _handle._cached = &_manager->assets.template create<AssetType>(std::forward<AssetTypeArgs>(args)...);
        _handle._refcount = detail::flag_refcount(&_handle._cached->refcount_control);
        return _handle;
    }

    handle_animation fetch_animation(const std::filesystem::path& path);
    handle_audio fetch_audio(const std::filesystem::path& path);
    handle_cubemap fetch_cubemap(const std::array<std::filesystem::path, 6>& paths, const std::optional<data_image_profile> profile = std::nullopt);
    handle_event_track fetch_event_track(const std::filesystem::path& path);
    handle_font fetch_font(context_window& window, const std::filesystem::path& path, const float32 font_size);
    handle_geometry fetch_geometry(const std::filesystem::path& path);
    handle_image fetch_image(const std::filesystem::path& path, const std::optional<data_image_profile> profile = std::nullopt);
    handle_mesh fetch_mesh(const std::filesystem::path& path);
    handle_motion_track fetch_motion_track(const std::filesystem::path& path);
    handle_shape fetch_shape(const std::filesystem::path& path, const detail::object_shape_algorithm algorithm = detail::object_shape_algorithm::triangle_mesh);
    handle_skeleton fetch_skeleton(const std::filesystem::path& path);
    handle_sound_track fetch_sound_track(const std::filesystem::path& path);
    handle_texture fetch_texture(const std::filesystem::path& path, const std::optional<data_image_profile> profile = std::nullopt);

	

    template <typename AssetType>
	using handle_user_asset = handle_asset<AssetType>;

    template <typename AssetType>
    handle_user_asset<AssetType> fetch_user_asset(const std::filesystem::path& path)
    {
        detail::assets_buffer<AssetType>& _assets = _manager->get_asset_buffer<AssetType>();
        const std::string _cache_id = path.string();

        detail::assets_cell<AssetType>* _cell = _assets.find_by_id(_cache_id);
        if (_cell == nullptr) {
            _cell = _assets.create_cell(detail::assets_async_slot<AssetType>::pending(AssetType {}), _cache_id);

            std::shared_ptr<detail::asset_fetch_context> _context = _manager->make_fetch_context();
            AssetType& _asset = _cell->fetched.emplaced_value();
            _context->fetch_as<AssetType>(path, [&_asset](AssetType&& loaded) {
                _asset = std::move(loaded);
            });
            _context->on_finished([_cell]() {
                _cell->fetched.mark_ready();
            });
            _context->close();
        }

        handle_user_asset<AssetType> _user_asset = {};
        _user_asset._cached = _cell;
        _user_asset._refcount = detail::flag_refcount(&_cell->refcount_control);
        return _user_asset;
    }

    //
    //
    //

    handle_shape create_shape_capsule(const float32 radius, const float32 height);

    /// @brief Creates a new geometry from a mesh
    /// @param mesh the mesh to create the geometry from
    /// @return a geometry object initialized with the mesh
    handle_geometry create_geometry(const handle_mesh mesh);

    /// @brief Creates a new geometry from a shape
    /// @param shape the shape to create the geometry from
    /// @return a geometry object initialized with the shape
    handle_geometry create_geometry(const handle_shape shape);

    /// @brief Creates a new image from a texture
    /// @param texture the texture to create the image from
    /// @return an image object initialized with the texture
    handle_image create_image(const handle_texture texture);

    /// @brief Creates a new image from a cubemap face
    /// @param cubemap the cubemap to create the image from
    /// @param face_index the index of the cubemap face to create the image from
    /// @return an image object initialized with the cubemap face
    handle_image create_image(const handle_cubemap cubemap, const uint32 face_index);

    /// @brief Creates a new texture from an image
    /// @param image the image to create the texture from
    /// @return a texture object initialized with the image
    handle_texture create_texture(const handle_image image);

    /// @brief Creates a new texture with the specified size
    /// @param size	the size of the texture to create
    /// @return a texture object with an empty texture of the specified size
    handle_texture create_texture(const uint32x2 size);

    template <typename AssetType, typename... AssetTypeArgs>
    handle_user_asset<AssetType> create_user_asset(AssetTypeArgs&&... args)
    {
        handle_user_asset<AssetType> _user_asset = {};
        _user_asset._cached = _manager->get_asset_buffer<AssetType>().create_cell(
            detail::assets_async_slot<AssetType>(AssetType(std::forward<AssetTypeArgs>(args)...)));
        _user_asset._refcount = detail::flag_refcount(&_user_asset._cached->refcount_control);
        return _user_asset;
    }

    //
    //
    //

    /// @brief Gets the current count of fetched objects that are still loading. You can use this to
    /// check if all the async fetches you issued are complete for loading screens.
    /// @return the current count of waiting async fetches
    uint32 async_fetches_waiting();

private:
    detail::manager_assets* _manager;
    friend struct access_context;
};

}
