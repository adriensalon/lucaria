#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/context_serialize.hpp>
#include <lucaria/engine/asset_geometry.hpp>

namespace lucaria {
namespace detail {

    asset_geometry::asset_geometry(const std::vector<char>& bytes)
        : origin(object_geometry_origin::path)
    {
        assets_bytes_stream _stream(bytes);
        cereal::PortableBinaryInputArchive _archive(_stream);
        _archive(data);
    }

    asset_geometry::asset_geometry(data_geometry&& data)
        : origin(object_geometry_origin::data)
        , data(std::move(data))
    {
    }

    void asset_geometry::save(context_save_storage& context) const
    {
        context.field("origin", origin);
        if (origin == object_geometry_origin::path) {
            context.field("origin_path", origin_path);
        }
        if (origin == object_geometry_origin::data) {
            context.field("origin_data", data);
        }
    }

    void asset_geometry::load(context_load_storage& context)
    {
        context.field("origin", origin);
        if (origin == object_geometry_origin::path) {
            context.field("origin_path", origin_path);
            const std::filesystem::path _path = origin_path;
            context.fetch(_path, [this, _path](const std::vector<char>& bytes) {
                *this = asset_geometry(bytes);
                origin_path = _path;
            });
        }
        if (origin == object_geometry_origin::data) {
            context.field("origin_data", data);
        }
    }

}
}
