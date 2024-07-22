#include <iostream>
#include <fstream>
#include <algorithm>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>

#include <core/material.hpp>
#include <core/fetch.hpp>

namespace detail {

static std::unordered_map<std::size_t, std::tuple<std::vector<material_texture>, std::unordered_map<material_texture, image_data>, std::promise<std::shared_ptr<material_ref>>>> promises;

}

material_ref::material_ref(const std::unordered_map<material_texture, image_data>& textures)
{
    for (const std::pair<const material_texture, image_data>& _pair : textures) {
        _textures.emplace(_pair.first, std::make_shared<texture_ref>(_pair.second));
    }
}

bool material_ref::get_has_texture(const material_texture texture)
{
    return (_textures.find(texture) != _textures.end());
}

texture_ref& material_ref::get_texture(const material_texture texture)
{
#if LUCARIA_DEBUG
    if (_textures.find(texture) == _textures.end()) {
        std::cout << "Impossible to retrieve material texture because it does not exist." << std::endl;
        std::terminate();
    }
#endif
    return *(_textures.at(texture).get());
}

std::shared_future<std::shared_ptr<material_ref>> fetch_material(const std::unordered_map<material_texture, std::filesystem::path>& texture_paths)
{
    std::vector<std::filesystem::path> _paths;
    std::transform(texture_paths.begin(), texture_paths.end(), std::back_inserter(_paths), [](const auto& _pair) { return _pair.second; });
    const std::size_t _hash = compute_hash_files(_paths);
    std::tuple<std::vector<material_texture>, std::unordered_map<material_texture, image_data>, std::promise<std::shared_ptr<material_ref>>>& _promise_tuple = detail::promises[_hash];
    std::vector<material_texture>& _keys = std::get<0>(_promise_tuple);
    for (const std::pair<const material_texture, std::filesystem::path>& _pair : texture_paths) {
        _keys.emplace_back(_pair.first);
    }
    std::unordered_map<material_texture, image_data>& _storage = std::get<1>(_promise_tuple);
    std::promise<std::shared_ptr<material_ref>>& _promise = std::get<2>(_promise_tuple);
    fetch_files(_paths, [&_keys, &_storage, &_promise](const std::size_t texture_index, const std::size_t textures_count, std::istringstream& stream) {
        const material_texture _texture_map = _keys[texture_index];
        _storage.emplace(_texture_map, std::move(load_texture_data(stream)));
        if (_storage.size() == textures_count) {
            _promise.set_value(std::move(std::make_shared<material_ref>(_storage)));
        }
    });
    return _promise.get_future();
}
