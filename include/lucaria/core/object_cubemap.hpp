#pragma once

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

    struct manager_assets;

    enum struct object_cubemap_origin {
        path,
        data
    };

    struct object_cubemap {
        object_cubemap() = default;
        object_cubemap(const object_cubemap& other) = delete;
        object_cubemap& operator=(const object_cubemap& other) = delete;
        object_cubemap(object_cubemap&& other) = default;
        object_cubemap& operator=(object_cubemap&& other) = default;
        ~object_cubemap();

        object_cubemap(const std::array<object_image, 6>& images);

        object_cubemap_origin origin;
        data_image_profile profile;
        std::array<std::filesystem::path, 6> origin_paths;

        template <typename ContextType>
        void save(ContextType& context) const
        {
            context(cereal::make_nvp("origin", origin));
            context(cereal::make_nvp("profile", profile));
            if (origin == object_cubemap_origin::path) {
                context(cereal::make_nvp("origin_path", origin_paths));
            }
        }

        template <typename ContextType>
        void load(ContextType& context)
        {
            context(cereal::make_nvp("origin", origin));
            context(cereal::make_nvp("profile", profile));
            if (origin == object_cubemap_origin::path) {
                context(cereal::make_nvp("origin_path", origin_paths));
                const std::array<std::filesystem::path, 6> _paths = origin_paths;
                const data_image_profile _profile = profile;
                const std::array<std::filesystem::path, 6> _resolved_paths = resolve_profile(context.objects, _paths, _profile);
                context.fetch(_resolved_paths, [this, _paths](const std::vector<std::vector<char>>& bytes) {
                    std::array<object_image, 6> _images = {
                        object_image(bytes[0]),
                        object_image(bytes[1]),
                        object_image(bytes[2]),
                        object_image(bytes[3]),
                        object_image(bytes[4]),
                        object_image(bytes[5])
                    };
                    *this = object_cubemap(_images);
                    origin_paths = _paths;
                });
            }
        }


#if defined(LUCARIA_BACKEND_OPENGL)
        flag_owning ownership = {};
        GLuint id = 0;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        flag_owning ownership = {};
        std::array<object_texture_pspgu, 6> faces = {};
#endif
    };

    [[nodiscard]] assets_cell<object_cubemap>& fetch(
        manager_assets& objects,
        assets_buffer<object_cubemap>& cached_vector,
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

    [[nodiscard]] recipe_object_cubemap make_recipe(const assets_cell<object_cubemap>& cache);
    [[nodiscard]] assets_cell<object_cubemap>* apply_recipe(manager_assets& objects, assets_buffer<object_cubemap>& cached, recipe_object_cubemap& recipe);

}
}
