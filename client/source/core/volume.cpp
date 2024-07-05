#include <core/volume.hpp>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

#include <glue/fetch.hpp>

namespace detail {

std::unordered_map<std::size_t, std::pair<std::vector<mesh_data>, std::promise<volume_data>>> volume_vector_promises;

}

volume_data::volume_data(const std::pair<glm::vec3, glm::vec3>& box)
{
    aabbs = { box };
}

volume_data::volume_data(const std::vector<std::pair<glm::vec3, glm::vec3>>& boxes)
{
    aabbs = boxes;
}

volume_data::volume_data(const mesh_data& mesh)
{
    glm::vec3 _min_values = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 _max_values = glm::vec3(std::numeric_limits<float>::lowest());
    // for (std::size_t _index = 0; _index < mesh.count; ++_index) {
    //     glm::vec3 vertex(
    //         mesh.positions[_index * 3],
    //         mesh.positions[_index * 3 + 1],
    //         mesh.positions[_index * 3 + 2]);
    //     _min_values = glm::min(_min_values, vertex);
    //     _max_values = glm::max(_max_values, vertex);
    // }
    aabbs = {{ _min_values, _max_values }};
}

volume_data::volume_data(const std::vector<mesh_data>& meshes)
{
    aabbs.clear();
    for (const mesh_data& _mesh : meshes) {        
        glm::vec3 _min_values = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 _max_values = glm::vec3(std::numeric_limits<float>::lowest());
        // for (std::size_t _index = 0; _index < _mesh.count; ++_index) {
        //     glm::vec3 _vertex(
        //         _mesh.positions[_index * 3],
        //         _mesh.positions[_index * 3 + 1],
        //         _mesh.positions[_index * 3 + 2]);
        //     _min_values = glm::min(_min_values, _vertex);
        //     _max_values = glm::max(_max_values, _vertex);
        // }
        aabbs.emplace_back(std::pair<glm::vec3, glm::vec3>{ _min_values, _max_values });
    }
}

bool get_is_contained(const volume_data& volume, const glm::vec3& position)
{
    for (const std::pair<glm::vec3, glm::vec3>& _aabb : volume.aabbs) {
        const glm::vec3 _min = _aabb.first;
        const glm::vec3 _max = _aabb.second;
        if (position.x >= _min.x && position.x <= _max.x && position.y >= _min.y && position.y <= _max.y && position.z >= _min.z && position.z <= _max.z) {
            return true;
        }
    }
    return false;
}

// volume_data load_volume(const std::vector<std::filesystem::path>& files)
// {
// #if LUCARIA_DEBUG
//     if (!std::filesystem::is_regular_file(file)) {
//         std::cout << "Invalid volume path " << file << std::endl;
//         std::terminate();
//     }
// #endif
//     volume_data _data;
//     {
//         cereal::PortableBinaryInputArchive _archive(stream);
//         _archive(_data.emplace_back());
//     }
// }

std::future<volume_data> fetch_volume(const std::vector<std::filesystem::path>& files)
{
    const std::size_t _hash = compute_hash_files(files);
    std::promise<volume_data>& _promise = detail::volume_vector_promises[_hash].second;
    std::vector<mesh_data>& _data = detail::volume_vector_promises[_hash].first;
    fetch_files(files, [&_data, &_promise, files, _hash](const std::size_t index, const std::size_t size, std::istringstream& stream) {
        {
            cereal::PortableBinaryInputArchive _archive(stream);
            _archive(_data.emplace_back());
        }
#if LUCARIA_DEBUG
        std::cout << "Loaded volume mesh data from " << files[index].generic_string() << " ("
                  << _data[index].count << " vertices)" << std::endl;
#endif
        if (_data.size() == size) {
            _promise.set_value(volume_data(_data));
        }
    });
    return _promise.get_future();
}