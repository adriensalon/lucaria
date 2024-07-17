#pragma once

#include <future>
#include <memory>

#include <core/mesh.hpp>
#include <core/material.hpp>

enum struct model_shader {
    blockout,
    unlit,
    pbr
};

template <model_shader shader_t = model_shader::unlit>
struct model_component {
    model_component() = default;
    model_component(const model_component& other) = delete;
    model_component& operator=(const model_component& other) = delete;
    model_component(model_component&& other) = default;
    model_component& operator=(model_component&& other) = default;

    model_component& material(const std::shared_future<std::shared_ptr<material_ref>>& fetched_material);
    model_component& mesh(const std::shared_future<std::shared_ptr<mesh_ref>>& fetched_mesh);

private:
    std::optional<std::shared_future<std::shared_ptr<material_ref>>> _fetched_material = std::nullopt;
    std::optional<std::shared_future<std::shared_ptr<mesh_ref>>> _fetched_mesh = std::nullopt;
    std::shared_ptr<material_ref> _material = nullptr;
    std::shared_ptr<mesh_ref> _mesh = nullptr;
    friend struct async_system;
    friend struct rendering_system;
};