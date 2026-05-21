#pragma once

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

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

    template <typename ImplementationType>
    struct implementation_save_ids {

        std::unordered_map<const implementation_container<ImplementationType>*, uint32> ids = {};
        uint32 next_id = 1;

        [[nodiscard]] uint32 get_or_create(const implementation_container<ImplementationType>* resource)
        {
            if (!resource) {
                return 0;
            }

            if (typename std::unordered_map<const implementation_container<ImplementationType>*, uint32>::const_iterator it = ids.find(resource); it != ids.end()) {
                return it->second;
            }

            const uint32 id = next_id++;
            ids.emplace(resource, id);
            return id;
        }
    };

    struct implementation_save_database {
        implementation_save_ids<image_implementation> images = {};
        implementation_save_ids<texture_implementation> textures = {};
        implementation_save_ids<cubemap_implementation> cubemaps = {};
        implementation_save_ids<geometry_implementation> geometries = {};
        implementation_save_ids<shape_implementation> shapes = {};
        implementation_save_ids<mesh_implementation> meshes = {};
        implementation_save_ids<font_implementation> fonts = {};
        implementation_save_ids<audio_implementation> audios = {};
        implementation_save_ids<sound_track_implementation> sound_tracks = {};
        implementation_save_ids<skeleton_implementation> skeletons = {};
        implementation_save_ids<animation_implementation> animations = {};
        implementation_save_ids<motion_track_implementation> motion_tracks = {};
        implementation_save_ids<event_track_implementation> event_tracks = {};
    };

    template <typename RecipeType>
    struct recipe_save_entry {
        uint32 save_id;
        RecipeType recipe;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("save_id", save_id));
            archive(cereal::make_nvp("recipe", recipe));
        }
    };

    struct recipe_save_database {
        std::vector<recipe_save_entry<image_recipe>> images = {};
        std::vector<recipe_save_entry<texture_recipe>> textures = {};
        std::vector<recipe_save_entry<cubemap_recipe>> cubemaps = {};
        std::vector<recipe_save_entry<geometry_recipe>> geometries = {};
        std::vector<recipe_save_entry<shape_recipe>> shapes = {};
        std::vector<recipe_save_entry<mesh_recipe>> meshes = {};
        std::vector<recipe_save_entry<font_recipe>> fonts = {};
        std::vector<recipe_save_entry<audio_recipe>> audios = {};
        std::vector<recipe_save_entry<sound_track_recipe>> sound_tracks = {};
        std::vector<recipe_save_entry<skeleton_recipe>> skeletons = {};
        std::vector<recipe_save_entry<animation_recipe>> animations = {};
        std::vector<recipe_save_entry<motion_track_recipe>> motion_tracks = {};
        std::vector<recipe_save_entry<event_track_recipe>> event_tracks = {};

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

    struct object_save_database {
        std::unordered_map<uint32, image_object> images = {};
        std::unordered_map<uint32, texture_object> textures = {};
        std::unordered_map<uint32, cubemap_object> cubemaps = {};
        std::unordered_map<uint32, geometry_object> geometries = {};
        std::unordered_map<uint32, shape_object> shapes = {};
        std::unordered_map<uint32, mesh_object> meshes = {};
        std::unordered_map<uint32, font_object> fonts = {};
        std::unordered_map<uint32, audio_object> audios = {};
        std::unordered_map<uint32, sound_track_object> sound_tracks = {};
        std::unordered_map<uint32, skeleton_object> skeletons = {};
        std::unordered_map<uint32, animation_object> animations = {};
        std::unordered_map<uint32, motion_track_object> motion_tracks = {};
        std::unordered_map<uint32, event_track_object> event_tracks = {};
    };

    struct implementation_database {
        implementation_manager<image_implementation> images = {};
        implementation_manager<texture_implementation> textures = {};
        implementation_manager<cubemap_implementation> cubemaps = {};
        implementation_manager<geometry_implementation> geometries = {};
        implementation_manager<shape_implementation> shapes = {};
        implementation_manager<mesh_implementation> meshes = {};
        implementation_manager<font_implementation> fonts = {};
        implementation_manager<audio_implementation> audios = {};
        implementation_manager<sound_track_implementation> sound_tracks = {};
        implementation_manager<skeleton_implementation> skeletons = {};
        implementation_manager<animation_implementation> animations = {};
        implementation_manager<motion_track_implementation> motion_tracks = {};
        implementation_manager<event_track_implementation> event_tracks = {};

        [[nodiscard]] recipe_save_database make_all_recipes(implementation_save_database& implementations) const;
        [[nodiscard]] object_save_database apply_recipes(recipe_save_database&& recipes);
    };

    implementation_database& engine_resources();

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

    /// @brief Creates a new geometry from a mesh
    /// @param mesh the mesh to create the geometry from
    /// @return a geometry object initialized with the mesh
    [[nodiscard]] geometry_object emplace_geometry(const mesh_object mesh);

    /// @brief Creates a new geometry from a shape
    /// @param shape the shape to create the geometry from
    /// @return a geometry object initialized with the shape
    [[nodiscard]] geometry_object emplace_geometry(const shape_object shape);

    /// @brief Creates a new image from a texture
    /// @param texture the texture to create the image from
    /// @return an image object initialized with the texture
    [[nodiscard]] image_object emplace_image(const texture_object texture);

    /// @brief Creates a new image from a cubemap face
    /// @param cubemap the cubemap to create the image from
    /// @param face_index the index of the cubemap face to create the image from
    /// @return an image object initialized with the cubemap face
    [[nodiscard]] image_object emplace_image(const cubemap_object cubemap, const uint32 face_index);

    /// @brief Creates a new texture from an image
    /// @param image the image to create the texture from
    /// @return a texture object initialized with the image
    [[nodiscard]] texture_object emplace_texture(const image_object image);

    /// @brief Creates a new texture with the specified size
    /// @param size	the size of the texture to create
    /// @return a texture object with an empty texture of the specified size
    [[nodiscard]] texture_object emplace_texture(const uint32x2 size);

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