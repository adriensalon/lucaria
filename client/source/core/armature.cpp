#include <fstream>
#include <iostream>
#include <unordered_map>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>

#include <core/armature.hpp>
#include <core/fetch.hpp>

namespace detail {
    
    static std::unordered_map<std::string, std::promise<std::shared_ptr<armature_ref>>> promises;

}

armature_ref::armature_ref(const armature_data& data)
{
    _vertices_count = data.count;
    _positions = data.positions;
    _bones = data.bones;
    _weights = data.weights;
}
const std::vector<glm::vec3>& armature_ref::get_positions() const
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

armature_data load_armature_data(std::istringstream& armature_stream)
{
    armature_data _data;
    {
#if LUCARIA_JSON        
        cereal::JSONInputArchive _archive(armature_stream);
#else
        cereal::PortableBinaryInputArchive _archive(armature_stream);
#endif
        _archive(_data);
    }
    return _data;
}

std::shared_future<std::shared_ptr<armature_ref>> fetch_armature(const std::filesystem::path& armature_path)
{
    std::promise<std::shared_ptr<armature_ref>>& _promise = detail::promises[armature_path.string()];
    fetch_file(armature_path, [&_promise](std::istringstream& stream) {
        _promise.set_value(std::move(std::make_shared<armature_ref>(load_armature_data(stream))));
    });
    return _promise.get_future();
}

