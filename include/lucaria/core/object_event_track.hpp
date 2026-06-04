#pragma once

#include <lucaria/bin/data_event_track.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/context_serialize.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

	struct manager_assets;

    enum struct object_event_track_origin {
        path,
		data
    };

    struct object_event_track {
        object_event_track() = default;
        object_event_track(const object_event_track& other) = delete;
        object_event_track& operator=(const object_event_track& other) = delete;
        object_event_track(object_event_track&& other) = default;
        object_event_track& operator=(object_event_track&& other) = default;

        object_event_track(const std::vector<char>& bytes);
        object_event_track(data_event_track&& data);

        object_event_track_origin origin;
		std::filesystem::path origin_path;		
        data_event_track data;

        void save(storage_save_context& context) const
        {
            context.field("origin", origin);
            if (origin == object_event_track_origin::path) {
                context.field("origin_path", origin_path);
            }
            if (origin == object_event_track_origin::data) {
                context.field("origin_data", data);
            }
        }

        void load(storage_load_context& context)
        {
            context.field("origin", origin);
            if (origin == object_event_track_origin::path) {
                context.field("origin_path", origin_path);
                const std::filesystem::path _path = origin_path;
                context.fetch(_path, [this, _path](const std::vector<char>& bytes) {
                    *this = object_event_track(bytes);
                    origin_path = _path;
                });
            }
            if (origin == object_event_track_origin::data) {
                context.field("origin_data", data);
            }
        }

    };

    [[nodiscard]] assets_cell<object_event_track>& fetch(
		manager_assets& objects, 
        assets_buffer<object_event_track>& cached_vector,
        const std::filesystem::path& path);

    // recipes

    struct recipe_object_event_track_path {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    struct recipe_object_event_track_data {
        data_event_track data;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
        }
    };

    using recipe_object_event_track = std::variant<recipe_object_event_track_path, recipe_object_event_track_data>;

    [[nodiscard]] recipe_object_event_track make_recipe(const assets_cell<object_event_track>& cache);
	[[nodiscard]] assets_cell<object_event_track>* apply_recipe(manager_assets& objects, assets_buffer<object_event_track>& cached, recipe_object_event_track& recipe);

}
}
