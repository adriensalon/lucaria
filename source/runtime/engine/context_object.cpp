#include <lucaria/engine/context_assets.hpp>
#include <lucaria/engine/context_window.hpp>

namespace lucaria {

handle_animation context_object::fetch_animation(const std::filesystem::path& path)
{
    return fetch_asset<detail::asset_animation, handle_animation>(
        path.string(),
        [path](detail::asset_animation& animation) {
            animation.origin_path = path;
        });
}

handle_audio context_object::fetch_audio(const std::filesystem::path& path)
{
    return fetch_asset<detail::asset_audio, handle_audio>(
        path.string(),
        [path](detail::asset_audio& audio) {
            audio.origin = detail::object_audio_origin::path;
            audio.origin_path = path;
        });
}

handle_cubemap context_object::fetch_cubemap(const std::array<std::filesystem::path, 6>& paths, const std::optional<data_image_profile> profile)
{
    const std::string _cache_id = paths[0].string(); // TODO include all paths and profile

    return fetch_asset<detail::asset_cubemap, handle_cubemap>(
        _cache_id,
        [paths, profile](detail::asset_cubemap& cubemap) {
            cubemap.origin = detail::asset_cubemap_origin::path;
            cubemap.origin_paths = paths;
            cubemap.cubemap.profile = profile.value_or(data_image_profile::rgba8888);
        });
}

handle_event_track context_object::fetch_event_track(const std::filesystem::path& path)
{
    return fetch_asset<detail::object_event_track, handle_event_track>(
        path.string(),
        [path](detail::object_event_track& event_track) {
            event_track.origin = detail::object_event_track_origin::path;
            event_track.origin_path = path;
        });
}

handle_font context_object::fetch_font(context_window& window, const std::filesystem::path& path, const float32 font_size)
{
    const std::string _cache_id = path.string(); // TODO include font_size if multiple sizes should coexist

    return fetch_asset<detail::object_font, handle_font>(
        _cache_id,
        [path, font_size](detail::object_font& font) {
            font.origin_path = path;
            font.font_size = font_size;
        },
        &window);
}

handle_geometry context_object::fetch_geometry(const std::filesystem::path& path)
{
    return fetch_asset<detail::asset_geometry, handle_geometry>(
        path.string(),
        [path](detail::asset_geometry& geometry) {
            geometry.origin = detail::object_geometry_origin::path;
            geometry.origin_path = path;
        });
}

handle_image context_object::fetch_image(const std::filesystem::path& path, const std::optional<data_image_profile> profile)
{
    return fetch_asset<detail::asset_image, handle_image>(
        path.string(),
        [path, profile](detail::asset_image& image) {
            image.origin = detail::object_image_origin::path;
            image.origin_path = path;
            image.profile = profile.value_or(data_image_profile {});
        });
}

handle_mesh context_object::fetch_mesh(const std::filesystem::path& path)
{
    return fetch_asset<detail::asset_mesh, handle_mesh>(
        path.string(),
        [path](detail::asset_mesh& mesh) {
            mesh.origin = detail::asset_mesh_origin::path;
            mesh.origin_path = path;
        });
}

handle_motion_track context_object::fetch_motion_track(const std::filesystem::path& path)
{
    return fetch_asset<detail::object_motion_track, handle_motion_track>(
        path.string(),
        [path](detail::object_motion_track& motion_track) {
            motion_track.origin = detail::object_motion_track_origin::path;
            motion_track.origin_path = path;
        });
}

handle_shape context_object::fetch_shape(const std::filesystem::path& path, const detail::object_shape_algorithm algorithm)
{
    const std::string _cache_id = path.string(); // TODO include algorithm if multiple algorithms should coexist

    return fetch_asset<detail::object_shape, handle_shape>(
        _cache_id,
        [path, algorithm](detail::object_shape& shape) {
            shape.origin = detail::object_shape_origin::path;
            shape.origin_path = path;
            shape.algorithm = algorithm;
        });
}

handle_skeleton context_object::fetch_skeleton(const std::filesystem::path& path)
{
    return fetch_asset<detail::object_skeleton, handle_skeleton>(
        path.string(),
        [path](detail::object_skeleton& skeleton) {
            skeleton.origin = detail::object_skeleton_origin::path;
            skeleton.origin_path = path;
        });
}

handle_sound_track context_object::fetch_sound_track(const std::filesystem::path& path)
{
    return fetch_asset<detail::object_sound_track, handle_sound_track>(
        path.string(),
        [path](detail::object_sound_track& sound_track) {
            sound_track.origin = detail::object_sound_track_origin::path;
            sound_track.origin_path = path;
        });
}

handle_texture context_object::fetch_texture(const std::filesystem::path& path, const std::optional<data_image_profile> profile)
{
    return fetch_asset<detail::asset_texture, handle_texture>(
        path.string(),
        [path, profile](detail::asset_texture& texture) {
            texture.origin = detail::object_texture_origin::path;
            texture.origin_path = path;
            // texture.profile = profile.value_or(data_image_profile {});
        });
}

handle_shape context_object::create_shape_capsule(const float32 radius, const float32 height)
{
    return create_asset<detail::object_shape, handle_shape>(new btCapsuleShape(radius, height), radius + height * 0.5f);
}

void context_object::set_prefix_path(const std::filesystem::path& prefix_path)
{
    _manager->async_prefix_path = prefix_path;
}

uint32 context_object::async_fetches_waiting()
{
    return _manager->async_fetches_waiting;
}

}
