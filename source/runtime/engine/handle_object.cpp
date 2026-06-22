#include <lucaria/engine/asset_font.hpp>
#include <lucaria/engine/asset_texture.hpp>
#include <lucaria/core/manager_assets.hpp>

namespace lucaria {

ImTextureID handle_texture::imgui_texture() const
{
    return _cached->fetched.value().imgui_texture();
}

ImVec2 handle_texture::imgui_uv0() const
{
    return _cached->fetched.value().imgui_uv0();
}

ImVec2 handle_texture::imgui_uv1() const
{
    return _cached->fetched.value().imgui_uv1();
}

ImFont* handle_font::imgui_font() const
{
    return _cached->fetched.value().font;
}

}
