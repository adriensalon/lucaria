#pragma once

#include <lucaria/core/object_animation.hpp>
#include <lucaria/core/object_cubemap.hpp>
#include <lucaria/core/object_event_track.hpp>
#include <lucaria/core/object_font.hpp>
#include <lucaria/core/object_mesh.hpp>
#include <lucaria/core/object_motion_track.hpp>
#include <lucaria/core/object_shape.hpp>
#include <lucaria/core/object_skeleton.hpp>
#include <lucaria/core/object_sound_track.hpp>
#include <lucaria/core/object_texture.hpp>

namespace lucaria {
namespace detail {

    struct manager_object {
        manager_object() = default;
        manager_object(const manager_object& other) = delete;
        manager_object& operator=(const manager_object& other) = delete;
        manager_object(manager_object&& other) = default;
        manager_object& operator=(manager_object&& other) = default;

        bool is_etc2_supported = false;
        bool is_s3tc_supported = false;
        std::atomic<uint32> async_fetches_waiting = 0;
        std::filesystem::path async_prefix_path = {};

        container_cache_vector<object_animation> animations = {};
        container_cache_vector<object_audio> audios = {};
        container_cache_vector<object_cubemap> cubemaps = {};
        container_cache_vector<object_event_track> event_tracks = {};
        container_cache_vector<object_font> fonts = {};
        container_cache_vector<object_geometry> geometries = {};
        container_cache_vector<object_image> images = {};
        container_cache_vector<object_mesh> meshes = {};
        container_cache_vector<object_motion_track> motion_tracks = {};
        container_cache_vector<object_shape> shapes = {};
        container_cache_vector<object_skeleton> skeletons = {};
        container_cache_vector<object_sound_track> sound_tracks = {};
        container_cache_vector<object_texture> textures = {};

        void load_bytes(const std::filesystem::path& path, const std::function<void(const std::vector<char>&)>& callback);
        void fetch_bytes(const std::filesystem::path& path, const std::function<void(const std::vector<char>&)>& callback, bool must_persist = true);
        void fetch_bytes(const std::vector<std::filesystem::path>& paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool must_persist = true);
        void fetch_bytes(const std::array<std::filesystem::path, 6>& paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool must_persist = true);
		void gc_unused();
    };

    // mappings

    struct mappings_manager_object_save {
        mappings_container_cache_vector<object_animation> animations = {};
        mappings_container_cache_vector<object_audio> audios = {};
        mappings_container_cache_vector<object_cubemap> cubemaps = {};
        mappings_container_cache_vector<object_event_track> event_tracks = {};
        mappings_container_cache_vector<object_font> fonts = {};
        mappings_container_cache_vector<object_geometry> geometries = {};
        mappings_container_cache_vector<object_image> images = {};
        mappings_container_cache_vector<object_mesh> meshes = {};
        mappings_container_cache_vector<object_motion_track> motion_tracks = {};
        mappings_container_cache_vector<object_shape> shapes = {};
        mappings_container_cache_vector<object_skeleton> skeletons = {};
        mappings_container_cache_vector<object_sound_track> sound_tracks = {};
        mappings_container_cache_vector<object_texture> textures = {};
    };

    struct mappings_manager_object_load {
        // std::unordered_map<uint32, image_ptr> load_images = {};
        // std::unordered_map<uint32, texture_ptr> load_textures = {};
        // std::unordered_map<uint32, cubemap_ptr> load_cubemaps = {};
        // std::unordered_map<uint32, geometry_ptr> load_geometries = {};
        // std::unordered_map<uint32, shape_ptr> load_shapes = {};
        // std::unordered_map<uint32, mesh_ptr> load_meshes = {};
        // std::unordered_map<uint32, font_ptr> load_fonts = {};
        // std::unordered_map<uint32, audio_ptr> load_audios = {};
        // std::unordered_map<uint32, sound_track_ptr> load_sound_tracks = {};
        // std::unordered_map<uint32, skeleton_ptr> load_skeletons = {};
        // std::unordered_map<uint32, animation_ptr> load_animations = {};
        // std::unordered_map<uint32, motion_track_ptr> load_motion_tracks = {};
        // std::unordered_map<uint32, event_track_ptr> load_event_tracks = {};
    };

    // recipes

    template <typename RecipeType>
    struct recipe_object_entry {
        uint32 save_id;
        RecipeType recipe;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("object_save_id", save_id));
            archive(cereal::make_nvp("recipe", recipe));
        }
    };

    struct recipe_manager_object {
        std::vector<recipe_object_entry<recipe_object_animation>> animations = {};
        std::vector<recipe_object_entry<recipe_object_audio>> audios = {};
        std::vector<recipe_object_entry<recipe_object_cubemap>> cubemaps = {};
        std::vector<recipe_object_entry<recipe_object_event_track>> event_tracks = {};
        std::vector<recipe_object_entry<recipe_object_font>> fonts = {};
        std::vector<recipe_object_entry<recipe_object_geometry>> geometries = {};
        std::vector<recipe_object_entry<recipe_object_image>> images = {};
        std::vector<recipe_object_entry<recipe_object_mesh>> meshes = {};
        std::vector<recipe_object_entry<recipe_object_motion_track>> motion_tracks = {};
        std::vector<recipe_object_entry<recipe_object_shape>> shapes = {};
        std::vector<recipe_object_entry<recipe_object_skeleton>> skeletons = {};
        std::vector<recipe_object_entry<recipe_object_sound_track>> sound_tracks = {};
        std::vector<recipe_object_entry<recipe_object_texture>> textures = {};

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("images", images));
            archive(cereal::make_nvp("textures", textures));
            archive(cereal::make_nvp("cubemaps", cubemaps));
            archive(cereal::make_nvp("geometries", geometries));
            archive(cereal::make_nvp("shapes", shapes));
            archive(cereal::make_nvp("meshes", meshes));
            archive(cereal::make_nvp("fonts", fonts));
            archive(cereal::make_nvp("audios", audios));
            archive(cereal::make_nvp("sound_tracks", sound_tracks));
            archive(cereal::make_nvp("skeletons", skeletons));
            archive(cereal::make_nvp("animations", animations));
            archive(cereal::make_nvp("motion_tracks", motion_tracks));
            archive(cereal::make_nvp("event_tracks", event_tracks));
        }
    };

    [[nodiscard]] recipe_manager_object make_recipe(const manager_object& objects, mappings_manager_object_save& mappings);

}
}
