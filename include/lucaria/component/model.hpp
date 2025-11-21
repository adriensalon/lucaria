#pragma once

#include <lucaria/core/mesh.hpp>
#include <lucaria/core/program.hpp>
#include <lucaria/core/texture.hpp>

namespace lucaria {
namespace detail {
    struct motion_system;
    struct rendering_system;
}

enum struct model_type {
    blockout,
    unlit,
    // pbr,
    // custom
};

template <model_type Type>
struct model_component;

template <>
struct model_component<model_type::blockout> {
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
struct model_component<model_type::unlit> {
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

///
/// NEXT
///

void set_lod_scale(const glm::float32 scale);

// struct model_component {
//     model_component() = default;
//     model_component(const model_component& other) = delete;
//     model_component& operator=(const model_component& other) = delete;
//     model_component(model_component&& other) = default;
//     model_component& operator=(model_component&& other) = default;

//     model_component& use_mesh(mesh& from);
//     model_component& use_mesh(fetched<mesh>& from);
    
// private:
//     detail::fetched_container<mesh> _mesh = {};
//     friend struct detail::motion_system;
//     friend struct detail::rendering_system;
// };

struct lod_model_component {
    lod_model_component() = default;
    lod_model_component(const lod_model_component& other) = delete;
    lod_model_component& operator=(const lod_model_component& other) = delete;
    lod_model_component(lod_model_component&& other) = default;
    lod_model_component& operator=(lod_model_component&& other) = default;

    lod_model_component& use_mesh(mesh& from, const glm::float32 minimum_distance, const glm::float32 crossfade_distance);
    lod_model_component& use_mesh(fetched<mesh>& from, const glm::float32 minimum_distance, const glm::float32 crossfade_distance);

private:
    std::vector<detail::fetched_container<mesh>> _meshes = {};
    std::vector<glm::float32> _minimum_distances = {};
    std::vector<glm::float32> _crossfade_distances = {};
    friend struct detail::motion_system;
    friend struct detail::rendering_system;
};

struct instanced_model_component {
    instanced_model_component() = default;
    instanced_model_component(const instanced_model_component& other) = delete;
    instanced_model_component& operator=(const instanced_model_component& other) = delete;
    instanced_model_component(instanced_model_component&& other) = default;
    instanced_model_component& operator=(instanced_model_component&& other) = default;

    instanced_model_component& use_mesh(mesh& from);
    instanced_model_component& use_mesh(fetched<mesh>& from);

    instanced_model_component& set_relative_transforms(std::vector<glm::mat4>&& from);

private:
    detail::fetched_container<mesh> _meshe = {};
    std::vector<glm::mat4> _relative_transforms = {};
    friend struct detail::motion_system;
    friend struct detail::rendering_system;
};

// constexpr std::size_t pbr_color_channels = 4;
// constexpr std::size_t pbr_normal_channels = 3;
// constexpr std::size_t pbr_occlusion_channels = 1;
// constexpr std::size_t pbr_roughness_channels = 1;
// constexpr std::size_t pbr_metallic_channels = 1;

// template <>
// struct model_component<model_type::pbr> {
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

using blockout_model_component = model_component<model_type::blockout>;
using unlit_model_component = model_component<model_type::unlit>;
// using pbr_model_component = model_component<model_shader::pbr>;
// using custom_model_component = model_component<model_type::custom>;

}
