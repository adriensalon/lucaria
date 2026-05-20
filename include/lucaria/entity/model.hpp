#pragma once

#include <lucaria/core/mesh.hpp>
#include <lucaria/core/program.hpp>
#include <lucaria/core/texture.hpp>

namespace lucaria {
namespace detail {
	struct rendering_system;
}

/// @brief
struct blockout_model_component {
    blockout_model_component() = default;
    blockout_model_component(const blockout_model_component& other) = delete;
    blockout_model_component& operator=(const blockout_model_component& other) = delete;
    blockout_model_component(blockout_model_component&& other) = default;
    blockout_model_component& operator=(blockout_model_component&& other) = default;

    blockout_model_component& use_mesh(const mesh_object mesh);

private:
    mesh_object _mesh = {};
    friend struct motion_system;
    friend struct detail::rendering_system;
};

/// @brief
struct unlit_model_component {
    unlit_model_component() = default;
    unlit_model_component(const unlit_model_component& other) = delete;
    unlit_model_component& operator=(const unlit_model_component& other) = delete;
    unlit_model_component(unlit_model_component&& other) = default;
    unlit_model_component& operator=(unlit_model_component&& other) = default;

    unlit_model_component& use_mesh(const mesh_object mesh);
    unlit_model_component& use_mesh_lod(const mesh_object mesh, const std::vector<std::pair<mesh_object, float32>>& lods);
    unlit_model_component& use_mesh_instanced(const mesh_object mesh, const std::vector<float32x4x4>& transforms);
    unlit_model_component& use_color(const texture_object color);

private:
    mesh_object _mesh = {};
    texture_object _color = {};
    friend struct motion_system;
    friend struct detail::rendering_system;
};

/// @brief
struct lit_model_component {
    lit_model_component() = default;
    lit_model_component(const lit_model_component& other) = delete;
    lit_model_component& operator=(const lit_model_component& other) = delete;
    lit_model_component(lit_model_component&& other) = default;
    lit_model_component& operator=(lit_model_component&& other) = default;

    lit_model_component& use_mesh(const mesh_object mesh);
    lit_model_component& use_mesh_lod(const mesh_object mesh, const std::vector<std::pair<mesh_object, float32>>& lods);
    lit_model_component& use_mesh_instanced(const mesh_object mesh, const std::vector<float32x4x4>& transforms);
    lit_model_component& use_color(const texture_object color);
    lit_model_component& use_normal(const texture_object normal);
    lit_model_component& use_metallic(const texture_object metallic);
    lit_model_component& use_roughness(const texture_object roughness);
    lit_model_component& use_occlusion(const texture_object occlusion);
    lit_model_component& use_emissive(const texture_object emissive);

private:
    mesh_object _mesh = {};
    texture_object _color = {};
    texture_object _normal = {};
    texture_object _metallic = {};
    texture_object _roughness = {};
    texture_object _occlusion = {};
    texture_object _emissive = {};
    friend struct motion_system;
    friend struct detail::rendering_system;
};

}
