#include <lucaria/ecs/component/model.hpp>

namespace lucaria {
namespace ecs {

    // blockout

    model_component<model_shader::blockout>& model_component<model_shader::blockout>::use_mesh(mesh& from)
    {
        _mesh.emplace(from);
        return *this;
    }

    model_component<model_shader::blockout>& model_component<model_shader::blockout>::use_mesh(fetched<mesh>& from)
    {
        _mesh.emplace(from);
        return *this;
    }

    // unlit

    model_component<model_shader::unlit>& model_component<model_shader::unlit>::use_mesh(mesh& from)
    {
        _mesh.emplace(from);
        return *this;
    }

    model_component<model_shader::unlit>& model_component<model_shader::unlit>::use_mesh(fetched<mesh>& from)
    {
        _mesh.emplace(from);
        return *this;
    }

    model_component<model_shader::unlit>& model_component<model_shader::unlit>::use_color(texture& from)
    {
        _color.emplace(from);
        return *this;
    }

    model_component<model_shader::unlit>& model_component<model_shader::unlit>::use_color(fetched<texture>& from)
    {
        _color.emplace(from);
        return *this;
    }

    // todo pbr

}
}
