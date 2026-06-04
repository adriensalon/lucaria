#pragma once

#include <lucaria/bin/data_image.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/context_serialize.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct object_cubemap;
    struct object_texture;
    struct manager_assets;

    [[nodiscard]] std::filesystem::path resolve_profile(
        manager_assets& objects,
        const std::filesystem::path& path,
        const std::optional<data_image_profile> profile = std::nullopt);

    [[nodiscard]] std::array<std::filesystem::path, 6> resolve_profile(
        manager_assets& objects,
        const std::array<std::filesystem::path, 6>& paths,
        const std::optional<data_image_profile> profile = std::nullopt);

    enum struct object_image_origin {
        path,
        data
    };

    struct object_image {
        object_image() = default;
        object_image(const object_image& other) = delete;
        object_image& operator=(const object_image& other) = delete;
        object_image(object_image&& other) noexcept = default;
        object_image& operator=(object_image&& other) noexcept = default;

        object_image(const std::vector<char>& bytes);
        object_image(data_image&& data);
        // object_image(const object_texture& texture);
        // object_image(const object_cubemap& cubemap, const uint32 face_index);

        object_image_origin origin;
		std::filesystem::path origin_path;		
		data_image_profile profile;
        data_image data;

        void save(storage_save_context& context) const
        {
            context.field("origin", origin);
            context.field("profile", profile);
            if (origin == object_image_origin::path) {
                context.field("origin_path", origin_path);
            }
            if (origin == object_image_origin::data) {
                context.field("origin_data", data);
            }
        }

        void load(storage_load_context& context)
        {
            context.field("origin", origin);
            context.field("profile", profile);
            if (origin == object_image_origin::path) {
                context.field("origin_path", origin_path);
                const std::filesystem::path _path = origin_path;
                const data_image_profile _profile = profile;
                const std::filesystem::path _resolved_path = resolve_profile(context.objects, _path, _profile);
                context.fetch(_resolved_path, [this, _path](const std::vector<char>& bytes) {
                    *this = object_image(bytes);
                    origin_path = _path;
                });
            }
            if (origin == object_image_origin::data) {
                context.field("origin_data", data);
            }
        }

    };
}
}
