#pragma once

#include <lucaria/core/animation.hpp>
#include <lucaria/core/cubemap.hpp>
#include <lucaria/core/event_track.hpp>
#include <lucaria/core/font.hpp>
#include <lucaria/core/mesh.hpp>
#include <lucaria/core/motion_track.hpp>
#include <lucaria/core/shape.hpp>
#include <lucaria/core/skeleton.hpp>
#include <lucaria/core/sound_track.hpp>
#include <lucaria/core/texture.hpp>

namespace lucaria {
namespace detail {

    struct resource_database {
        resource_manager<image_implementation> images = {};
        resource_manager<texture_implementation> textures = {};
        resource_manager<cubemap_implementation> cubemaps = {};
        resource_manager<geometry_implementation> geometries = {};
        resource_manager<shape_implementation> shapes = {};
        resource_manager<mesh_implementation> meshes = {};
        resource_manager<font_implementation> fonts = {};
        resource_manager<audio_implementation> audios = {};
        resource_manager<sound_track_implementation> sound_tracks = {};
        resource_manager<skeleton_implementation> skeletons = {};
        resource_manager<animation_implementation> animations = {};
        resource_manager<motion_track_implementation> motion_tracks = {};
        resource_manager<event_track_implementation> event_tracks = {};
    };

    resource_database& engine_resources();

}

struct object_context {

	/// @brief 
	/// @param path 
	/// @return 
	[[nodiscard]] animation_object fetch_animation(const std::filesystem::path& path);

	/// @brief 
	/// @param path 
	/// @return 
	[[nodiscard]] audio_object fetch_audio(const std::filesystem::path& path);



    /// @brief Creates a new texture with the specified size
    /// @param size	the size of the texture to create
    /// @return a texture object with an empty texture of the specified size
    [[nodiscard]] texture_object create_texture(const uint32x2 size);

    /// @brief Loads an image from a file asynchronously and uploads directly to the device,
    /// lets the runtime choose the best format it can use without downloading the others
    /// @param path path to load uncompressed image version from
    /// @param etc2_path path to load ETC2 compressed image version from
    /// @param s3tc_path path to load S3TC compressed image version from
    [[nodiscard]] texture_object fetch_texture(
        const std::filesystem::path& path,
        const std::optional<std::filesystem::path>& etc2_path = std::nullopt,
        const std::optional<std::filesystem::path>& s3tc_path = std::nullopt);
};

}