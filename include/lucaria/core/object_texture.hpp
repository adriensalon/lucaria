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

#include <lucaria/core/context_serialize.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct manager_assets;

    enum struct object_texture_origin {
        path,
        data,
        size
    };

    struct object_texture {
        object_texture() = default;
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
        std::filesystem::path origin_path;

        void save(storage_save_context& context) const
        {
            context.field("origin", origin);
            context.field("profile", profile);
            if (origin == object_texture_origin::path) {
                context.field("origin_path", origin_path);
            }
            if (origin == object_texture_origin::size) {
                context.field("size", size);
            }
        }

        void load(storage_load_context& context)
        {
            context.field("origin", origin);
            context.field("profile", profile);
            if (origin == object_texture_origin::path) {
                context.field("origin_path", origin_path);
                const std::filesystem::path _path = origin_path;
                const data_image_profile _profile = profile;
                const std::filesystem::path _resolved_path = resolve_profile(context.objects, _path, _profile);
                context.fetch(_resolved_path, [this, _path](const std::vector<char>& bytes) {
                    object_image _image(bytes);
                    *this = object_texture(_image);
                    origin_path = _path;
                });
            }
            if (origin == object_texture_origin::size) {
                context.field("size", size);
                *this = object_texture(size);
            }
        }


        data_image_profile profile;
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

    [[nodiscard]] assets_cell<object_texture>& fetch(
        manager_assets& objects,
        assets_buffer<object_texture>& cache_vector,
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

    [[nodiscard]] recipe_object_texture make_recipe(const assets_cell<object_texture>& cache);
    [[nodiscard]] assets_cell<object_texture>* apply_recipe(manager_assets& objects, assets_buffer<object_texture>& cached, recipe_object_texture& recipe);

}
}
