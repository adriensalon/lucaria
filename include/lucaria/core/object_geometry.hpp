#pragma once

#include <lucaria/bin/data_geometry.hpp>
#include <lucaria/core/utils_cache.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

    struct object_shape;
    struct object_mesh;
	struct manager_object;

    enum struct object_geometry_origin {
        path,
        data
    };

    struct object_geometry {
        LUCARIA_DELETE_DEFAULT(object_geometry)
        object_geometry(const object_geometry& other) = delete;
        object_geometry& operator=(const object_geometry& other) = delete;
        object_geometry(object_geometry&& other) = default;
        object_geometry& operator=(object_geometry&& other) = default;

        object_geometry(const std::vector<char>& bytes);
        // object_geometry(const object_mesh& mesh);
        // object_geometry(const object_shape& shape);
        object_geometry(data_geometry&& data);

        object_geometry_origin origin;
        data_geometry data;
    };

    [[nodiscard]] container_cache<object_geometry>& fetch(
		manager_object& objects, 
        container_cache_vector<object_geometry>& cached_vector,
		const std::filesystem::path& path);

	// recipes

    struct recipe_object_geometry_path {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    struct recipe_object_geometry_data {
        data_geometry data;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
        }
    };

    using recipe_object_geometry = std::variant<recipe_object_geometry_path, recipe_object_geometry_data>;

    [[nodiscard]] recipe_object_geometry make_recipe(const container_cache<object_geometry>& cache);

}
}
