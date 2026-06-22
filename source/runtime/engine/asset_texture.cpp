#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/asset_texture.hpp>

namespace lucaria {
namespace detail {

    asset_texture::asset_texture(const asset_image& image)
        : origin(image.origin == object_image_origin::path ? object_texture_origin::path : object_texture_origin::data)
        , texture(*textures_registry, image.data)
    {
        LUCARIA_DEBUG_ASSERT(textures_registry, "asset_texture::textures_registry is null")
    }

    asset_texture::asset_texture(const uint32x2 size)
        : origin(object_texture_origin::size)
        , texture(size)
    {
		LUCARIA_DEBUG_ASSERT(textures_registry, "asset_texture::textures_registry is null")
    }

    void asset_texture::resize(const uint32x2 new_size)
    {
        origin = object_texture_origin::size;
        texture.resize(new_size);
    }

    void asset_texture::update(const asset_image& from)
    {
        origin = object_texture_origin::data;
        texture.update(from.data);
    }

    ImTextureID asset_texture::imgui_texture() const
    {
        return texture.imgui_texture();
    }

    ImVec2 asset_texture::imgui_uv0() const
    {
        return texture.imgui_uv0();
    }

    ImVec2 asset_texture::imgui_uv1() const
    {
        return texture.imgui_uv1();
    }

    void asset_texture::save(storage_save_context& context) const
    {
        context.field("origin", origin);
        context.field("profile", texture.profile);
        if (origin == object_texture_origin::path) {
            context.field("origin_path", origin_path);
        }
        if (origin == object_texture_origin::size) {
            context.field("size", texture.size);
        }
    }

    void asset_texture::load(storage_load_context& context)
    {
        context.field("origin", origin);
        context.field("profile", texture.profile);
        if (origin == object_texture_origin::path) {
            context.field("origin_path", origin_path);
            const std::filesystem::path _path = origin_path;
            const data_image_profile _profile = texture.profile;
            const std::filesystem::path _resolved_path = resolve_profile(context.objects, _path, _profile);
            context.fetch(_resolved_path, [this, _path](const std::vector<char>& _bytes) {
                asset_image _image(_bytes);
                *this = asset_texture(_image);
                origin_path = _path;
            });
        }
        if (origin == object_texture_origin::size) {
            context.field("size", texture.size);
            *this = asset_texture(texture.size);
        }
    }

}
}
