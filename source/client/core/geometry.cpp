#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/geometry.hpp>

namespace lucaria {
namespace {

    static void load_data_from_bytes(geometry_data& data, const std::vector<char>& data_bytes)
    {
        detail::bytes_stream _stream(data_bytes);
#if LUCARIA_JSON
        cereal::JSONInputArchive _archive(_stream);
#else
        cereal::PortableBinaryInputArchive _archive(_stream);
#endif
        _archive(data);
    }
    
}

geometry::geometry(geometry_data&& data)
{
    this->data = std::move(data);
}

geometry::geometry(const std::vector<char>& data_bytes)
{
    load_data_from_bytes(data, data_bytes);
}

geometry::geometry(const std::filesystem::path& data_path)
{
    detail::load_bytes(data_path, [this](const std::vector<char>& _data_bytes) {
        load_data_from_bytes(data, _data_bytes);
    });
}

fetched<geometry> fetch_geometry(const std::filesystem::path& data_path)
{
    std::shared_ptr<std::promise<geometry>> _promise = std::make_shared<std::promise<geometry>>();
    detail::fetch_bytes(data_path, [_promise](const std::vector<char>& _data_bytes) {
        geometry _geometry(_data_bytes);
        _promise->set_value(std::move(_geometry));
    });

    return fetched<geometry>(_promise->get_future());
}

}