#pragma once

#include <imgui.h>

#include <lucaria/core/object_image.hpp>
#include <lucaria/core/utils_owning.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend_pspgu.hpp>
#endif

namespace lucaria {
namespace detail {
	
    struct manager_object;

    enum struct object_texture_origin {
        path,
        data,
        size
    };

    struct object_texture {
        LUCARIA_DELETE_DEFAULT(object_texture)
        object_texture(const object_texture& other) = delete;
        object_texture& operator=(const object_texture& other) = delete;
        object_texture(object_texture&& other) = default;
        object_texture& operator=(object_texture&& other) = default;
        ~object_texture();

        object_texture(const object_image& image);
        object_texture(const uint32x2 size);
        void resize(const uint32x2 new_size);
        void update(const object_image& image);
        [[nodiscard]] ImTextureID imgui_texture() const;

        object_texture_origin origin;
        uint32x2 size;

#if defined(LUCARIA_BACKEND_OPENGL)
        flag_owning ownership = {};
        GLuint id = 0;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        bool is_owning = false;
        void* pixels = nullptr; // VRAM or memalign(16)
        int psm = GU_PSM_8888;
        int tbw = 0; // texture buffer width
        bool is_swizzled = false;
#endif
    };

    [[nodiscard]] container_cache<object_texture>& fetch(
		manager_object& objects,
        container_cache_vector<object_texture>& cache_vector,
        const std::filesystem::path& path,
        const std::optional<data_image_profile> profile = std::nullopt);

    // recipes

    struct recipe_object_texture_path {
        std::filesystem::path path;
        data_image_profile profile;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
            archive(cereal::make_nvp("profile", profile));
        }
    };

    struct recipe_object_texture_data {
        data_image data;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
        }
    };

    struct recipe_object_texture_size {
        uint32x2 size;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("size", size));
        }
    };

    using recipe_object_texture = std::variant<recipe_object_texture_path, recipe_object_texture_data, recipe_object_texture_size>;

    [[nodiscard]] recipe_object_texture make_recipe(const container_cache<object_texture>& cache);

}
}
