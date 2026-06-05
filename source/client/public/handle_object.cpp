#include <lucaria/engine/asset_font.hpp>
#include <lucaria/engine/asset_texture.hpp>
#include <lucaria/core/manager_assets.hpp>

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
