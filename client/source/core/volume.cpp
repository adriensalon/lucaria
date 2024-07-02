#include <core/volume.hpp>

namespace detail {
    
}

bool get_is_contained(const volume_data& volume, const glm::vec3& point);

std::optional<float> get_distance(const volume_data& volume, const glm::vec3& axis, const glm::vec3& point);

volume_data load_volume(const std::vector<std::filesystem::path>& files)
{

}

std::future<volume_data> fetch_volume(const std::vector<std::filesystem::path>& files)
{
    std::promise<mesh_data>& _promise = detail::promises[file.generic_string()];
    fetch_file(file.generic_string(), [&_promise, file](std::istringstream& stream) {
        mesh_data _data;
        {
            cereal::PortableBinaryInputArchive _archive(stream);
            _archive(_data);
        }
#if LUCARIA_DEBUG
        std::cout << "Loaded mesh data from " << file << " ("
                  << _data.count << " vertices)" << std::endl;
#endif
        _promise.set_value(std::move(_data));
    });
    return _promise.get_future();
}