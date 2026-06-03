#pragma once

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/public/handle_animation.hpp>
#include <lucaria/public/handle_audio.hpp>
#include <lucaria/public/handle_cubemap.hpp>
#include <lucaria/public/handle_event_track.hpp>
#include <lucaria/public/handle_font.hpp>
#include <lucaria/public/handle_geometry.hpp>
#include <lucaria/public/handle_image.hpp>
#include <lucaria/public/handle_mesh.hpp>
#include <lucaria/public/handle_motion_track.hpp>
#include <lucaria/public/handle_shape.hpp>
#include <lucaria/public/handle_skeleton.hpp>
#include <lucaria/public/handle_sound_track.hpp>
#include <lucaria/public/handle_texture.hpp>
#include <lucaria/public/handle_user_asset.hpp>

namespace lucaria {

struct context_window;

/// @brief
struct context_object {

    /// @brief
    /// @param path
    /// @return
    handle_animation fetch_animation(const std::filesystem::path& path);

    /// @brief
    /// @param path
    /// @return
    handle_audio fetch_audio(const std::filesystem::path& path);

    /// @brief
    /// @param paths
    /// @param profile
    /// @return
    handle_cubemap fetch_cubemap(const std::array<std::filesystem::path, 6>& paths, const std::optional<data_image_profile> profile = std::nullopt);

    /// @brief
    /// @param path
    /// @return
    handle_event_track fetch_event_track(const std::filesystem::path& path);

    /// @brief
    /// @param path
    /// @param font_size
    /// @return
    handle_font fetch_font(context_window& window, const std::filesystem::path& path, const float32 font_size);

    /// @brief
    /// @param path
    /// @return
    handle_geometry fetch_geometry(const std::filesystem::path& path);

    /// @brief
    /// @param paths
    /// @param profile
    /// @return
    handle_image fetch_image(const std::filesystem::path& path, const std::optional<data_image_profile> profile = std::nullopt);

    /// @brief
    /// @param path
    /// @return
    handle_mesh fetch_mesh(const std::filesystem::path& path);

    /// @brief
    /// @param path
    /// @return
    handle_motion_track fetch_motion_track(const std::filesystem::path& path);

    /// @brief
    /// @param path
    /// @return
    handle_shape fetch_shape(const std::filesystem::path& path, const detail::object_shape_algorithm algorithm = detail::object_shape_algorithm::triangle_mesh);

    /// @brief
    /// @param path
    /// @return
    handle_skeleton fetch_skeleton(const std::filesystem::path& path);

    /// @brief
    /// @param path
    /// @return
    handle_sound_track fetch_sound_track(const std::filesystem::path& path);

    /// @brief Loads an image from a file asynchronously and uploads directly to the device,
    /// lets the runtime choose the best format it can use without downloading the others
    /// @param path path to load uncompressed image version from
    /// @param profile
    handle_texture fetch_texture(const std::filesystem::path& path, const std::optional<data_image_profile> profile = std::nullopt);

    template <typename AssetType>
    handle_user_asset<AssetType> fetch_user_asset(const std::filesystem::path& path)
    {
        handle_user_asset<AssetType> _user_asset = {};
        _user_asset._cached = &detail::fetch(*_manager, _manager->get_user_asset_storage<AssetType>().assets, path);
        _user_asset._refcount = detail::flag_refcount(&_user_asset._cached->refcount_control);
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
        _user_asset._cached = _manager->get_user_asset_storage<AssetType>().assets.create_cell();
        _user_asset._cached->fetched = detail::object_user_asset<AssetType>(std::forward<AssetTypeArgs>(args)...);
        _user_asset._refcount = detail::flag_refcount(&_user_asset._cached->refcount_control);
        return _user_asset;
    }

    //
    //
    //

    /// @brief Sets a prefix for all the subsequent uses of the filesystem. You can use this if
    /// you build for multiple platforms and choose different storage path strategies.
    /// @param prefix_path the prefix path to use for subsequent filesystem accesses
    void set_prefix_path(const std::filesystem::path& prefix_path);

    /// @brief Gets the current count of fetched objects that are still loading. You can use this to
    /// check if all the async fetches you issued are complete for loading screens.
    /// @return the current count of waiting async fetches
    uint32 async_fetches_waiting();

private:
    detail::manager_assets* _manager;
    friend struct access_context;
};

}