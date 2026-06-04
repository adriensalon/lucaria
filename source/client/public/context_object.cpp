#include <lucaria/public/context_object.hpp>
#include <lucaria/public/context_window.hpp>

namespace lucaria {
namespace {

    template <typename AssetType, typename Configure>
    detail::assets_cell<AssetType>& fetch_with_serialization_context(
        detail::manager_assets& objects,
        detail::assets_buffer<AssetType>& buffer,
        const std::string& cache_id,
        Configure&& configure,
        detail::manager_window* window = nullptr)
    {
        if (detail::assets_cell<AssetType>* existing = buffer.find_by_id(cache_id)) {
            return *existing;
        }

        detail::assets_cell<AssetType>* cell = buffer.create_cell(
            detail::container_async<AssetType>::pending(AssetType {}),
            cache_id);

        AssetType& asset = cell->fetched.emplaced_value();
        std::forward<Configure>(configure)(asset);

        std::shared_ptr<detail::runtime_storage_context<AssetType>> context =
            objects.make_runtime_storage_context<AssetType>(cell, asset, window);
        detail::load_user_asset_value(asset, *context);
        context->close();

        return *cell;
    }

    template <typename HandleType, typename CellType>
    HandleType make_handle(CellType& cell)
    {
        HandleType handle = {};
        handle._cached = &cell;
        handle._refcount = detail::flag_refcount(&cell.refcount_control);
        return handle;
    }

}

handle_animation context_object::fetch_animation(const std::filesystem::path& path)
{
    detail::assets_cell<detail::object_animation>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->animations,
        path.string(),
        [path](detail::object_animation& animation) {
            animation.origin = detail::object_animation_origin::path;
            animation.origin_path = path;
        });

    return make_handle<handle_animation>(cell);
}

handle_audio context_object::fetch_audio(const std::filesystem::path& path)
{
    detail::assets_cell<detail::object_audio>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->audios,
        path.string(),
        [path](detail::object_audio& audio) {
            audio.origin = detail::object_audio_origin::path;
            audio.origin_path = path;
        });

    return make_handle<handle_audio>(cell);
}

handle_cubemap context_object::fetch_cubemap(const std::array<std::filesystem::path, 6>& paths, const std::optional<data_image_profile> profile)
{
    const std::string cache_id = paths[0].string(); // TODO concatenate all paths and profile

    detail::assets_cell<detail::object_cubemap>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->cubemaps,
        cache_id,
        [paths, profile](detail::object_cubemap& cubemap) {
            cubemap.origin = detail::object_cubemap_origin::path;
            cubemap.origin_paths = paths;
            cubemap.profile = profile.value_or(data_image_profile {});
        });

    return make_handle<handle_cubemap>(cell);
}

handle_event_track context_object::fetch_event_track(const std::filesystem::path& path)
{
    detail::assets_cell<detail::object_event_track>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->event_tracks,
        path.string(),
        [path](detail::object_event_track& event_track) {
            event_track.origin = detail::object_event_track_origin::path;
            event_track.origin_path = path;
        });

    return make_handle<handle_event_track>(cell);
}

handle_font context_object::fetch_font(context_window& window, const std::filesystem::path& path, const float32 font_size)
{
    const std::string cache_id = path.string(); // TODO include font_size if multiple sizes should coexist

    detail::assets_cell<detail::object_font>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->fonts,
        cache_id,
        [path, font_size](detail::object_font& font) {
            font.origin = detail::object_font_origin::path;
            font.origin_path = path;
            font.font_size = font_size;
        },
        window._manager);

    return make_handle<handle_font>(cell);
}

handle_geometry context_object::fetch_geometry(const std::filesystem::path& path)
{
    detail::assets_cell<detail::object_geometry>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->geometries,
        path.string(),
        [path](detail::object_geometry& geometry) {
            geometry.origin = detail::object_geometry_origin::path;
            geometry.origin_path = path;
        });

    return make_handle<handle_geometry>(cell);
}

handle_image context_object::fetch_image(const std::filesystem::path& path, const std::optional<data_image_profile> profile)
{
    detail::assets_cell<detail::object_image>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->images,
        path.string(),
        [path, profile](detail::object_image& image) {
            image.origin = detail::object_image_origin::path;
            image.origin_path = path;
            image.profile = profile.value_or(data_image_profile {});
        });

    return make_handle<handle_image>(cell);
}

handle_mesh context_object::fetch_mesh(const std::filesystem::path& path)
{
    detail::assets_cell<detail::object_mesh>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->meshes,
        path.string(),
        [path](detail::object_mesh& mesh) {
            mesh.origin = detail::object_mesh_origin::path;
            mesh.origin_path = path;
        });

    return make_handle<handle_mesh>(cell);
}

handle_motion_track context_object::fetch_motion_track(const std::filesystem::path& path)
{
    detail::assets_cell<detail::object_motion_track>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->motion_tracks,
        path.string(),
        [path](detail::object_motion_track& motion_track) {
            motion_track.origin = detail::object_motion_track_origin::path;
            motion_track.origin_path = path;
        });

    return make_handle<handle_motion_track>(cell);
}

handle_shape context_object::fetch_shape(const std::filesystem::path& path, const detail::object_shape_algorithm algorithm)
{
    const std::string cache_id = path.string(); // TODO include algorithm if multiple algorithms should coexist

    detail::assets_cell<detail::object_shape>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->shapes,
        cache_id,
        [path, algorithm](detail::object_shape& shape) {
            shape.origin = detail::object_shape_origin::path;
            shape.origin_path = path;
            shape.algorithm = algorithm;
        });

    return make_handle<handle_shape>(cell);
}

handle_skeleton context_object::fetch_skeleton(const std::filesystem::path& path)
{
    detail::assets_cell<detail::object_skeleton>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->skeletons,
        path.string(),
        [path](detail::object_skeleton& skeleton) {
            skeleton.origin = detail::object_skeleton_origin::path;
            skeleton.origin_path = path;
        });

    return make_handle<handle_skeleton>(cell);
}

handle_sound_track context_object::fetch_sound_track(const std::filesystem::path& path)
{
    detail::assets_cell<detail::object_sound_track>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->sound_tracks,
        path.string(),
        [path](detail::object_sound_track& sound_track) {
            sound_track.origin = detail::object_sound_track_origin::path;
            sound_track.origin_path = path;
        });

    return make_handle<handle_sound_track>(cell);
}

handle_texture context_object::fetch_texture(const std::filesystem::path& path, const std::optional<data_image_profile> profile)
{
    detail::assets_cell<detail::object_texture>& cell = fetch_with_serialization_context(
        *_manager,
        _manager->textures,
        path.string(),
        [path, profile](detail::object_texture& texture) {
            texture.origin = detail::object_texture_origin::path;
            texture.origin_path = path;
            texture.profile = profile.value_or(data_image_profile {});
        });

    return make_handle<handle_texture>(cell);
}

//
//

handle_shape context_object::create_shape_capsule(const float32 radius, const float32 height)
{
    handle_shape _shape = {};
    _shape._cached = _manager->shapes.create_cell();
    _shape._cached->fetched = detail::object_shape(new btCapsuleShape(radius, height), radius + height * 0.5f);
    _shape._refcount = detail::flag_refcount(&_shape._cached->refcount_control);
    return _shape;
}

//
//

void context_object::set_prefix_path(const std::filesystem::path& prefix_path)
{
    _manager->async_prefix_path = prefix_path;
}

uint32 context_object::async_fetches_waiting()
{
    return _manager->async_fetches_waiting;
}

}
