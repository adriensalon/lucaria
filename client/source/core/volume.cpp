#include <core/volume.hpp>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>

#include <core/fetch.hpp>

namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<volume_ref>>> promises;

}

volume_ref::volume_ref(const volume_data& data)
{
    _minimum = data.minimum;
    _maximum = data.maximum;
}

bool volume_ref::get_is_contained(const glm::vec3& position)
{
    return (position.x >= _minimum.x && position.x <= _maximum.x
        && position.y >= _minimum.y && position.y <= _maximum.y
        && position.z >= _minimum.z && position.z <= _maximum.z);
}

volume_data load_volume_data(std::istringstream& volume_stream)
{
    volume_data _data;
    {
#if LUCARIA_JSON        
        cereal::JSONInputArchive _archive(volume_stream);
#else
        cereal::PortableBinaryInputArchive _archive(volume_stream);
#endif
        _archive(_data);
    }
    return _data;
}

std::shared_future<std::shared_ptr<volume_ref>> fetch_volume(const std::filesystem::path& volume_path)
{
    std::promise<std::shared_ptr<volume_ref>>& _promise = detail::promises[volume_path.string()];
    fetch_file(volume_path, [&_promise](std::istringstream& stream) {
        _promise.set_value(std::move(std::make_shared<volume_ref>(load_volume_data(stream))));
    });
    return _promise.get_future();
}
