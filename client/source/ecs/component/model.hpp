#pragma once

#include <future>
#include <memory>

#include <core/fetch.hpp>
#include <core/mesh.hpp>
#include <core/material.hpp>

enum struct model_shader {
    blockout,
    unlit,
    pbr
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
    friend struct rendering_system;
};

template <>
struct model_component<model_shader::pbr> {
    model_component() = default;
    model_component(const model_component& other) = delete;
    model_component& operator=(const model_component& other) = delete;
    model_component(model_component&& other) = default;
    model_component& operator=(model_component&& other) = default;

    model_component& material(const std::shared_future<std::shared_ptr<material_ref>>& fetched_material);
    model_component& mesh(const std::shared_future<std::shared_ptr<mesh_ref>>& fetched_mesh);

private:
    fetch_container<material_ref> _material = {};
    fetch_container<mesh_ref> _mesh = {};
    friend struct rendering_system;
};

using blockout_model_component = model_component<model_shader::blockout>;
using unlit_model_component = model_component<model_shader::unlit>;
using pbr_model_component = model_component<model_shader::pbr>;