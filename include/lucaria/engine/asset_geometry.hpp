#pragma once

#include <lucaria/bin/data_geometry.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/serialize_context.hpp>
#include <lucaria/forward/handle_asset.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct object_shape;
    struct object_mesh;
    struct manager_assets;

    enum struct object_geometry_origin {
        path,
        data
    };

    struct object_geometry {
        object_geometry() = default;
        object_geometry(const object_geometry& other) = delete;
        object_geometry& operator=(const object_geometry& other) = delete;
        object_geometry(object_geometry&& other) = default;
        object_geometry& operator=(object_geometry&& other) = default;

        object_geometry(const std::vector<char>& bytes);
        // object_geometry(const object_mesh& mesh);
        // object_geometry(const object_shape& shape);
        object_geometry(data_geometry&& data);

        object_geometry_origin origin;
        std::filesystem::path origin_path;
        data_geometry data;

        void save(storage_save_context& context) const
        {
            context.field("origin", origin);
            if (origin == object_geometry_origin::path) {
                context.field("origin_path", origin_path);
            }
            if (origin == object_geometry_origin::data) {
                context.field("origin_data", data);
            }
        }

        void load(storage_load_context& context)
        {
            context.field("origin", origin);
            if (origin == object_geometry_origin::path) {
                context.field("origin_path", origin_path);
                const std::filesystem::path _path = origin_path;
                context.fetch(_path, [this, _path](const std::vector<char>& bytes) {
                    *this = object_geometry(bytes);
                    origin_path = _path;
                });
            }
            if (origin == object_geometry_origin::data) {
                context.field("origin_data", data);
            }
        }
    };
}

struct handle_geometry : handle_asset<detail::object_geometry> {
    using handle_asset<detail::object_geometry>::handle_asset;
};

}
