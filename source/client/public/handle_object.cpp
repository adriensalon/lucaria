#include <lucaria/public/handle_font.hpp>
#include <lucaria/public/handle_texture.hpp>

namespace lucaria {

ImTextureID handle_texture::imgui_texture() const
{
    return _cached->fetched.value().imgui_texture();
}

ImFont* handle_font::imgui_font() const
{
    return _cached->fetched.value().font;
}

}
