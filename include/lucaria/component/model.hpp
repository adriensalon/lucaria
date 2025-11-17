#pragma once

#include <lucaria/core/mesh.hpp>
#include <lucaria/core/texture.hpp>

namespace lucaria {
namespace detail {
    struct motion_system;
    struct rendering_system;
}

enum struct model_shader {
    blockout,
    unlit,
    pbr,
    custom
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

    model_component& use_mesh(mesh& from);
    model_component& use_mesh(fetched<mesh>& from);

private:
    detail::fetched_container<mesh> _mesh = {};
    friend struct detail::motion_system;
    friend struct detail::rendering_system;
};

template <>
struct model_component<model_shader::unlit> {
    model_component() = default;
    model_component(const model_component& other) = delete;
    model_component& operator=(const model_component& other) = delete;
    model_component(model_component&& other) = default;
    model_component& operator=(model_component&& other) = default;

    model_component& use_mesh(mesh& from);
    model_component& use_mesh(fetched<mesh>& from);
    model_component& use_color(texture& from);
    model_component& use_color(fetched<texture>& from);

private:
    detail::fetched_container<mesh> _mesh = {};
    detail::fetched_container<texture> _color = {};
    friend struct detail::motion_system;
    friend struct detail::rendering_system;
};

// constexpr std::size_t pbr_color_channels = 4;
// constexpr std::size_t pbr_normal_channels = 3;
// constexpr std::size_t pbr_occlusion_channels = 1;
// constexpr std::size_t pbr_roughness_channels = 1;
// constexpr std::size_t pbr_metallic_channels = 1;

// template <>
// struct model_component<model_shader::pbr> {
//     model_component() = default;
//     model_component(const model_component& other) = delete;
//     model_component& operator=(const model_component& other) = delete;
//     model_component(model_component&& other) = default;
//     model_component& operator=(model_component&& other) = default;

//     model_component& use_color(texture& color);
//     model_component& use_metallic(texture& metallic);
//     model_component& use_roughness(texture& roughness);
//     model_component& use_normal(texture& normal);
//     model_component& use_occlusion(texture& occlusion);
//     model_component& use_emissive(texture& emissive);
//     model_component& use_height(texture& height);
//     model_component& use_mesh(mesh& value, geometry& data);

// private:
//     std::optional<std::reference_wrapper<texture>> _color = std::nullopt;
//     std::optional<std::reference_wrapper<texture>> _metallic = std::nullopt;
//     std::optional<std::reference_wrapper<texture>> _roughness = std::nullopt;
//     std::optional<std::reference_wrapper<texture>> _normal = std::nullopt;
//     std::optional<std::reference_wrapper<texture>> _occlusion = std::nullopt;
//     std::optional<std::reference_wrapper<texture>> _emissive = std::nullopt;
//     std::optional<std::reference_wrapper<texture>> _height = std::nullopt;
//     std::optional<std::reference_wrapper<mesh>> _mesh = std::nullopt;
//     friend struct motion_system;
//     friend struct rendering_system;
// };

template <>
struct model_component<model_shader::custom> {
    model_component() = default;
    model_component(const model_component& other) = delete;
    model_component& operator=(const model_component& other) = delete;
    model_component(model_component&& other) = default;
    model_component& operator=(model_component&& other) = default;

    model_component& use_program(program& from);
    model_component& use_program(fetched<program>& from);
    model_component& use_mesh(const std::vector<std::pair<std::string, mesh_attribute>>& named_attributes, mesh& from);
    model_component& use_mesh(const std::vector<std::pair<std::string, mesh_attribute>>& named_attributes, fetched<mesh>& from);
    model_component& use_cubemap(const std::string& name, cubemap& from, const glm::uint slot);
    model_component& use_cubemap(const std::string& name, fetched<cubemap>& from, const glm::uint slot);
    model_component& use_texture(const std::string& name, texture& from, const glm::uint slot);
    model_component& use_texture(const std::string& name, fetched<texture>& from, const glm::uint slot);

private:
    // TODO
    friend struct detail::motion_system;
    friend struct detail::rendering_system;
};

using blockout_model_component = model_component<model_shader::blockout>;
using unlit_model_component = model_component<model_shader::unlit>;
// using pbr_model_component = model_component<model_shader::pbr>;
using custom_model_component = model_component<model_shader::custom>;

}
