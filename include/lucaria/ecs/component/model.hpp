#pragma once

#include <functional>
#include <future>
#include <memory>

#include <imgui.h>

#include <lucaria/core/fetch.hpp>
#include <lucaria/core/font.hpp>
#include <lucaria/core/framebuffer.hpp>
#include <lucaria/core/mesh.hpp>
#include <lucaria/core/texture.hpp>
#include <lucaria/core/world.hpp>

namespace lucaria {

enum struct model_shader {
    blockout,
    unlit,
    imgui,
    // pbr
};

template <model_shader shader_t>
struct model_component;

template <>
struct model_component<model_shader::blockout> {
    model_component() = default;
    model_component(const model_component& other) = delete;
    model_component& operator=(const model_component& other) = delete;
    model_component(model_component&& other) = default;
    model_component& operator=(model_component&& other) = default;

    model_component& mesh(const std::shared_future<std::shared_ptr<mesh_ref>>& fetched_mesh);

private:
    fetch_container<mesh_ref> _mesh = {};
    friend struct motion_system;
    friend struct rendering_system;
};

template <>
struct model_component<model_shader::unlit> {
    model_component() = default;
    model_component(const model_component& other) = delete;
    model_component& operator=(const model_component& other) = delete;
    model_component(model_component&& other) = default;
    model_component& operator=(model_component&& other) = default;

    model_component& color(const std::shared_future<std::shared_ptr<texture_ref>>& fetched_texture);
    model_component& mesh(const std::shared_future<std::shared_ptr<mesh_ref>>& fetched_mesh);

private:
    fetch_container<texture_ref> _color = {};
    fetch_container<mesh_ref> _mesh = {};
    friend struct motion_system;
    friend struct rendering_system;
};

// enum struct pbr_texture {
//     color,
//     normal,
//     occlusion,
//     roughness,
//     metallic
// };

// inline const std::unordered_map<pbr_texture, std::size_t> pbr_texture_channels = {
//     { pbr_texture::color, 4 },
//     { pbr_texture::normal, 3 },
//     { pbr_texture::occlusion, 1 },
//     { pbr_texture::roughness, 1 },
//     { pbr_texture::metallic, 1 },
// };

// template <>
// struct model_component<model_shader::pbr> {
//     model_component() = default;
//     model_component(const model_component& other) = delete;
//     model_component& operator=(const model_component& other) = delete;
//     model_component(model_component&& other) = default;
//     model_component& operator=(model_component&& other) = default;

//     // model_component& textures(const std::unordered_map<pbr_texture, std::shared_future<std::shared_ptr<texture_ref>>>& fetched_material);

//     model_component& color(const std::shared_future<std::shared_ptr<texture_ref>>& fetched_texture);
//     model_component& metallic(const std::shared_future<std::shared_ptr<texture_ref>>& fetched_texture);
//     model_component& roughness(const std::shared_future<std::shared_ptr<texture_ref>>& fetched_texture);
//     model_component& normal(const std::shared_future<std::shared_ptr<texture_ref>>& fetched_texture);
//     model_component& occlusion(const std::shared_future<std::shared_ptr<texture_ref>>& fetched_texture);
//     model_component& emissive(const std::shared_future<std::shared_ptr<texture_ref>>& fetched_texture);
//     model_component& height(const std::shared_future<std::shared_ptr<texture_ref>>& fetched_texture);
//     model_component& mesh(const std::shared_future<std::shared_ptr<mesh_ref>>& fetched_mesh);

// private:
//     fetch_container<texture_ref> _color = {};
//     fetch_container<texture_ref> _metallic = {};
//     fetch_container<texture_ref> _roughness = {};
//     fetch_container<texture_ref> _normal = {};
//     fetch_container<texture_ref> _occlusion = {};
//     fetch_container<texture_ref> _emissive = {};
//     fetch_container<texture_ref> _height = {};
//     fetch_container<mesh_ref> _mesh = {};
//     friend struct motion_system;
//     friend struct rendering_system;
// };

using blockout_model_component = model_component<model_shader::blockout>;
using unlit_model_component = model_component<model_shader::unlit>;
using imgui_model_component = model_component<model_shader::imgui>;
// using pbr_model_component = model_component<model_shader::pbr>;

}
