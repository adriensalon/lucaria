#pragma once

#include <lucaria/bin/data_geometry.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

    struct object_shape;
    struct object_mesh;
	struct manager_assets;

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
		std::filesystem::path origin_path;		
        data_geometry data;

        template <typename Archive>
        void serialize(Archive& archive)
        {
            archive(cereal::make_nvp("origin", origin));
            if (origin == object_geometry_origin::path) {
                archive(cereal::make_nvp("origin_path", origin_path));
            }
            if (origin == object_geometry_origin::data) {
                archive(cereal::make_nvp("origin_data", data));
            }
        }
    };

    [[nodiscard]] assets_cell<object_geometry>& fetch(
		manager_assets& objects, 
        assets_buffer<object_geometry>& cached_vector,
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

    [[nodiscard]] recipe_object_geometry make_recipe(const assets_cell<object_geometry>& cache);
	[[nodiscard]] assets_cell<object_geometry>* apply_recipe(manager_assets& objects, assets_buffer<object_geometry>& cached, recipe_object_geometry& recipe);

}
}
