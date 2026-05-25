#include <lucaria/public/handle_animation.hpp>
#include <lucaria/public/handle_audio.hpp>
#include <lucaria/public/handle_cubemap.hpp>
#include <lucaria/public/handle_event_track.hpp>
#include <lucaria/public/handle_font.hpp>
#include <lucaria/public/handle_geometry.hpp>
#include <lucaria/public/handle_image.hpp>
#include <lucaria/public/handle_mesh.hpp>
#include <lucaria/public/handle_motion_track.hpp>
#include <lucaria/public/handle_shape.hpp>
#include <lucaria/public/handle_skeleton.hpp>
#include <lucaria/public/handle_sound_track.hpp>
#include <lucaria/public/handle_texture.hpp>

namespace lucaria {

#define LUCARIA_HANDLE_IMPLEMENTATION(ObjectName)                     \
    bool handle_##ObjectName::has_value() const                       \
    {                                                                 \
        return _cached && _cached->fetched.has_value();               \
    }                                                                 \
    handle_##ObjectName::operator bool() const                        \
    {                                                                 \
        return has_value();                                           \
    }

LUCARIA_HANDLE_IMPLEMENTATION(animation)
LUCARIA_HANDLE_IMPLEMENTATION(audio)
LUCARIA_HANDLE_IMPLEMENTATION(cubemap)
LUCARIA_HANDLE_IMPLEMENTATION(event_track)
LUCARIA_HANDLE_IMPLEMENTATION(font)
LUCARIA_HANDLE_IMPLEMENTATION(geometry)
LUCARIA_HANDLE_IMPLEMENTATION(image)
LUCARIA_HANDLE_IMPLEMENTATION(mesh)
LUCARIA_HANDLE_IMPLEMENTATION(motion_track)
LUCARIA_HANDLE_IMPLEMENTATION(shape)
LUCARIA_HANDLE_IMPLEMENTATION(skeleton)
LUCARIA_HANDLE_IMPLEMENTATION(sound_track)
LUCARIA_HANDLE_IMPLEMENTATION(texture)

ImTextureID handle_texture::imgui_texture() const
{
    return _cached->fetched.value().imgui_texture();
}

ImFont* handle_font::imgui_font() const
{
    return _cached->fetched.value().font;
}

}
