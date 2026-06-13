#pragma once

#include <lucaria/bin/data_image.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/serialize_context.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct asset_cubemap;
    struct asset_texture;
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

    struct asset_image {
        asset_image() = default;
        asset_image(const asset_image& other) = delete;
        asset_image& operator=(const asset_image& other) = delete;
        asset_image(asset_image&& other) noexcept = default;
        asset_image& operator=(asset_image&& other) noexcept = default;

        asset_image(const std::vector<char>& bytes);
        asset_image(data_image&& data);
        // asset_image(const asset_texture& texture);
        // asset_image(const asset_cubemap& cubemap, const uint32 face_index);

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
                    *this = asset_image(bytes);
                    origin_path = _path;
                });
            }
            if (origin == object_image_origin::data) {
                context.field("origin_data", data);
            }
        }
    };
}

struct handle_image : handle_asset<detail::asset_image> {
    using handle_asset<detail::asset_image>::handle_asset;

    /// @brief Resizes the image to the specified size
    /// @param size the new size of the image
    void resize(const uint32x2 size);

    /// @brief Gets the size of the image
    /// @return the size of the image
    uint32x2 get_size() const;
};

}
