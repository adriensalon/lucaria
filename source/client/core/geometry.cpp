#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/database.hpp>
#include <lucaria/core/geometry.hpp>
#include <lucaria/core/stream.hpp>
#include <lucaria/core/fetch.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static void _load_geometry_bytes(geometry_data& data, const std::vector<char>& bytes)
        {
            bytes_stream _stream(bytes);
#if defined(LUCARIA_JSON_ASSETS)
            cereal::JSONInputArchive _archive(_stream);
#else
            cereal::PortableBinaryInputArchive _archive(_stream);
#endif
            _archive(data);
        }

        static async_container<geometry_implementation> _fetch_geometry_async(const std::filesystem::path& data_path)
        {
            std::shared_ptr<std::promise<geometry_implementation>> _promise = std::make_shared<std::promise<geometry_implementation>>();
            fetch_bytes(data_path, [_promise](const std::vector<char>& _bytes) {
        geometry_implementation _geometry(_bytes);
        _promise->set_value(std::move(_geometry)); }, true);

            return async_container<geometry_implementation>(_promise->get_future());
        }
    }

    geometry_implementation::geometry_implementation(const std::vector<char>& bytes)
    {
        _load_geometry_bytes(data, bytes);
    }

    geometry_implementation::geometry_implementation(geometry_data&& data)
        : data(std::move(data))
    {
    }

}

geometry_object geometry_object::fetch(const std::filesystem::path& path)
{
    detail::resource_container<detail::geometry_implementation>* _resource = detail::engine_resources().geometries.get_or_create_by_path(path, [&] {
        return detail::_fetch_geometry_async(path);
    });

    return geometry_object { _resource };
}

bool geometry_object::has_value() const
{
    return _resource && _resource->is_ready();
}

geometry_object::operator bool() const
{
    return has_value();
}

geometry_object::geometry_object(detail::resource_container<detail::geometry_implementation>* resource)
    : _resource(resource)
{
}

}