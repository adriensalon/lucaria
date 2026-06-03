#include <lucaria/public/context_object.hpp>
#include <lucaria/public/context_window.hpp>

namespace lucaria {

handle_animation context_object::fetch_animation(const std::filesystem::path& path)
{
    handle_animation _animation = {};
    _animation._cached = &detail::fetch(*_manager, _manager->animations, path);
    _animation._refcount = detail::flag_refcount(&_animation._cached->refcount_control);
    return _animation;
}

handle_audio context_object::fetch_audio(const std::filesystem::path& path)
{
    handle_audio _audio = {};
    _audio._cached = &detail::fetch(*_manager, _manager->audios, path);
    _audio._refcount = detail::flag_refcount(&_audio._cached->refcount_control);
    return _audio;
}

handle_cubemap context_object::fetch_cubemap(const std::array<std::filesystem::path, 6>& paths, const std::optional<data_image_profile> profile)
{
    handle_cubemap _cubemap = {};
    _cubemap._cached = &detail::fetch(*_manager, _manager->cubemaps, paths, profile);
    _cubemap._refcount = detail::flag_refcount(&_cubemap._cached->refcount_control);
    return _cubemap;
}

handle_event_track context_object::fetch_event_track(const std::filesystem::path& path)
{
    handle_event_track _event_track = {};
    _event_track._cached = &detail::fetch(*_manager, _manager->event_tracks, path);
    _event_track._refcount = detail::flag_refcount(&_event_track._cached->refcount_control);
    return _event_track;
}

handle_font context_object::fetch_font(context_window& window, const std::filesystem::path& path, const float32 font_size)
{
    handle_font _font = {};
    _font._cached = &detail::fetch(*window._manager, *_manager, _manager->fonts, path, font_size);
    _font._refcount = detail::flag_refcount(&_font._cached->refcount_control);
    return _font;
}

handle_geometry context_object::fetch_geometry(const std::filesystem::path& path)
{
    handle_geometry _geometry = {};
    _geometry._cached = &detail::fetch(*_manager, _manager->geometries, path);
    _geometry._refcount = detail::flag_refcount(&_geometry._cached->refcount_control);
    return _geometry;
}

handle_image context_object::fetch_image(const std::filesystem::path& path, const std::optional<data_image_profile> profile)
{
    handle_image _image = {};
    _image._cached = &detail::fetch(*_manager, _manager->images, path, profile);
    _image._refcount = detail::flag_refcount(&_image._cached->refcount_control);
    return _image;
}

handle_mesh context_object::fetch_mesh(const std::filesystem::path& path)
{
    handle_mesh _mesh = {};
    _mesh._cached = &detail::fetch(*_manager, _manager->meshes, path);
    _mesh._refcount = detail::flag_refcount(&_mesh._cached->refcount_control);
    return _mesh;
}

handle_motion_track context_object::fetch_motion_track(const std::filesystem::path& path)
{
    handle_motion_track _motion_track = {};
    _motion_track._cached = &detail::fetch(*_manager, _manager->motion_tracks, path);
    _motion_track._refcount = detail::flag_refcount(&_motion_track._cached->refcount_control);
    return _motion_track;
}

handle_shape context_object::fetch_shape(const std::filesystem::path& path, const detail::object_shape_algorithm algorithm)
{
    handle_shape _shape = {};
    _shape._cached = &detail::fetch(*_manager, _manager->shapes, path, algorithm);
    _shape._refcount = detail::flag_refcount(&_shape._cached->refcount_control);
    return _shape;
}

handle_skeleton context_object::fetch_skeleton(const std::filesystem::path& path)
{
    handle_skeleton _skeleton = {};
    _skeleton._cached = &detail::fetch(*_manager, _manager->skeletons, path);
    _skeleton._refcount = detail::flag_refcount(&_skeleton._cached->refcount_control);
    return _skeleton;
}

handle_sound_track context_object::fetch_sound_track(const std::filesystem::path& path)
{
    handle_sound_track _sound_track = {};
    _sound_track._cached = &detail::fetch(*_manager, _manager->sound_tracks, path);
    _sound_track._refcount = detail::flag_refcount(&_sound_track._cached->refcount_control);
    return _sound_track;
}

handle_texture context_object::fetch_texture(const std::filesystem::path& path, const std::optional<data_image_profile> profile)
{
    handle_texture _texture = {};
    _texture._cached = &detail::fetch(*_manager, _manager->textures, path, profile);
    _texture._refcount = detail::flag_refcount(&_texture._cached->refcount_control);
    return _texture;
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