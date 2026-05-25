#pragma once

#include <lucaria/public/handle_mesh.hpp>
#include <lucaria/public/handle_texture.hpp>

namespace lucaria {

/// @brief
struct component_model_blockout {
    component_model_blockout() = default;
    component_model_blockout(const component_model_blockout& other) = delete;
    component_model_blockout& operator=(const component_model_blockout& other) = delete;
    component_model_blockout(component_model_blockout&& other) = default;
    component_model_blockout& operator=(component_model_blockout&& other) = default;

    component_model_blockout& use_mesh(const handle_mesh mesh);

private:
    handle_mesh _mesh = {};

    template <typename ArchiveType>
    void serialize(ArchiveType& archive)
    {
        archive(cereal::make_nvp("mesh", _mesh));
    }

    friend struct detail::system_rendering;
	friend class cereal::access;
};

/// @brief
struct component_model_unlit {
    component_model_unlit() = default;
    component_model_unlit(const component_model_unlit& other) = delete;
    component_model_unlit& operator=(const component_model_unlit& other) = delete;
    component_model_unlit(component_model_unlit&& other) = default;
    component_model_unlit& operator=(component_model_unlit&& other) = default;

    component_model_unlit& use_mesh(const handle_mesh mesh);
    component_model_unlit& use_color(const handle_texture color);

private:
    handle_mesh _mesh = {};
    handle_texture _color = {};

    template <typename ArchiveType>
    void serialize(ArchiveType& archive)
    {
        archive(cereal::make_nvp("mesh", _mesh));
        archive(cereal::make_nvp("color", _color));
    }

    friend struct detail::system_rendering;
	friend class cereal::access;
};

}
