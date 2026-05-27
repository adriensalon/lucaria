#pragma once

#include <lucaria/core/utils_owning.hpp>
#include <lucaria/core/object_image.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend_pspgu.hpp>
#endif

namespace lucaria {
namespace detail {

	struct manager_object;

    enum struct object_cubemap_origin {
        path,
        data
    };

    struct object_cubemap {
        LUCARIA_DELETE_DEFAULT(object_cubemap)
        object_cubemap(const object_cubemap& other) = delete;
        object_cubemap& operator=(const object_cubemap& other) = delete;
        object_cubemap(object_cubemap&& other) = default;
        object_cubemap& operator=(object_cubemap&& other) = default;
        ~object_cubemap();

        object_cubemap(const std::array<object_image, 6>& images);

        object_cubemap_origin origin;

#if defined(LUCARIA_BACKEND_OPENGL)
        flag_owning ownership = {};
        GLuint id = 0;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        flag_owning ownership = {};
        std::array<object_texture_pspgu, 6> faces = {};
#endif
    };

    [[nodiscard]] container_cache<object_cubemap>& fetch(
		manager_object& objects,
        container_cache_vector<object_cubemap>& cached_vector,
        const std::array<std::filesystem::path, 6>& paths,
        const std::optional<data_image_profile> profile = std::nullopt);

    // recipes

    struct recipe_object_cubemap_path {
        std::array<std::filesystem::path, 6> paths;
        data_image_profile profile;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("paths", paths));
            archive(cereal::make_nvp("profile", profile));
        }
    };

    struct recipe_object_cubemap_data {
        std::array<data_image, 6> datas;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("datas", datas));
        }
    };

    using recipe_object_cubemap = std::variant<recipe_object_cubemap_path, recipe_object_cubemap_data>;

    [[nodiscard]] recipe_object_cubemap make_recipe(const container_cache<object_cubemap>& cache);
	[[nodiscard]] container_cache<object_cubemap>* apply_recipe(manager_object& objects, container_cache_vector<object_cubemap>& cached, recipe_object_cubemap& recipe);

}
}
