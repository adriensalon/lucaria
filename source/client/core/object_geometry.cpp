#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/asset_geometry.hpp>
#include <lucaria/core/assets_stream.hpp>

namespace lucaria {
namespace detail {

    object_geometry::object_geometry(const std::vector<char>& bytes)
        : origin(object_geometry_origin::path)
    {
        bytes_stream _stream(bytes);
        cereal::PortableBinaryInputArchive _archive(_stream);
        _archive(data);
    }

    object_geometry::object_geometry(data_geometry&& data)
        : origin(object_geometry_origin::data)
        , data(std::move(data))
    {
    }

}
}
