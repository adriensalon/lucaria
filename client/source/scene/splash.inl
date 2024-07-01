#pragma once

#include <type_traits>
#include <unordered_map>

#include <core/program.hpp>
#include <glue/fetch.hpp>

namespace detail {

void update_splash(const std::chrono::seconds& duration, const std::unordered_map<std::string, std::pair<int, int>>& infos);

template <typename resource_t>
void emplace_resource(std::future<resource_t>& resource, std::unordered_map<std::string, std::pair<int, int>>& infos)
{
    std::string _name;
    if constexpr (std::is_same_v<resource_t, mesh_data>) {
        _name = "meshes";
    } else if constexpr (std::is_same_v<resource_t, texture_data>) {
        _name = "textures";
    } else if constexpr (std::is_same_v<resource_t, shader_data>) {
        _name = "shaders";
    } else {
        _name = typeid(resource_t).name();
    }
    if (infos.find(_name) == infos.end()) {
        infos.emplace(_name, std::pair<int, int> { 0, 0 });
    }
    infos.at(_name).second++;
    if (is_future_ready(resource)) {
        infos.at(_name).first++;
    }
}

}

template <typename... resources_t>
void update_splash(const std::chrono::seconds& duration, std::future<resources_t>&... resources)
{
    std::unordered_map<std::string, std::pair<int, int>> _infos;
    (detail::emplace_resource(resources, _infos), ...);
    detail::update_splash(duration, _infos);
}