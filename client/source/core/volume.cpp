#include <core/volume.hpp>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

#include <glue/fetch.hpp>

namespace detail {
    
std::unordered_map<std::size_t, std::pair<std::vector<mesh_data>, std::promise<volume_data>>> volume_vector_promises;

}

volume_data::volume_data(const std::pair<glm::vec3, glm::vec3>& aabb)
{

}

volume_data::volume_data(const std::vector<std::pair<glm::vec3, glm::vec3>>& aabbs)
{

}

volume_data::volume_data(const mesh_data& mesh)
{

}

volume_data::volume_data(const std::vector<mesh_data>& meshes)
{

}

bool get_is_contained(const volume_data& volume, const glm::vec3& point)
{
    return false;
}

std::optional<float> get_distance(const volume_data& volume, const glm::vec3& axis, const glm::vec3& point)
{
    return std::nullopt;
}

// volume_data load_volume(const std::vector<std::filesystem::path>& files)
// {

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