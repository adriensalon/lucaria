#pragma once

#include <type_traits>
#include <unordered_map>

#include <core/program.hpp>
#include <glue/fetch.hpp>

namespace detail {

void update_splash(const std::chrono::seconds& duration, const std::size_t loaded, const std::size_t total);

template <typename resource_t>
void emplace_resource(std::future<resource_t>& resource, std::unordered_map<std::string, std::pair<std::size_t, std::size_t>>& infos)
{
    std::string _name = typeid(resource_t).name();
    if (infos.find(_name) == infos.end()) {
        infos.emplace(_name, std::pair<std::size_t, std::size_t> { 0, 0 });
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
    std::unordered_map<std::string, std::pair<std::size_t, std::size_t>> _infos;
    (detail::emplace_resource(resources, _infos), ...);
    std::size_t _loaded = 0;
    std::size_t _total = 0;
    for (const std::pair<std::string, std::pair<std::size_t, std::size_t>> _info : _infos) {
        _loaded += _info.second.first;
        _total += _info.second.second;
    }
    detail::update_splash(duration, _loaded, _total);
}