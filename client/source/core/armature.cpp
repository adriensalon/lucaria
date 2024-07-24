#include <fstream>
#include <iostream>
#include <unordered_map>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>

#include <core/armature.hpp>
#include <core/fetch.hpp>
#include <core/mesh.hpp>

namespace detail {
    
    static std::unordered_map<std::string, std::promise<std::shared_ptr<armature_ref>>> promises;

}

armature_ref::armature_ref(const geometry_data& data)
{
    _vertices_count = data.count;
    _positions = data.positions;
    _bones = data.bones;
    _weights = data.weights;
}

std::vector<glm::vec3>& armature_ref::get_positions()
{
    return _positions;
}

const std::vector<glm::uvec4>& armature_ref::get_bones() const
{
    return _bones;
}

const std::vector<glm::vec4>& armature_ref::get_weights() const
{
    return _weights;
}

glm::uint armature_ref::get_vertices_count() const
{
    return _vertices_count;
}

std::shared_future<std::shared_ptr<armature_ref>> fetch_armature(const std::filesystem::path& armature_path)
{
    std::promise<std::shared_ptr<armature_ref>>& _promise = detail::promises[armature_path.string()];
    fetch_file(armature_path, [&_promise](const std::vector<char>& geometry_bytes) {
        _promise.set_value(std::move(std::make_shared<armature_ref>(load_geometry_data(geometry_bytes))));
    });
    return _promise.get_future();
}

