#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/object_event_track.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    object_event_track::object_event_track(const std::vector<char>& bytes)
        : origin(object_event_track_origin::path)
    {
        bytes_stream _stream(bytes);
        cereal::PortableBinaryInputArchive _archive(_stream);
        _archive(data);
    }

    object_event_track::object_event_track(data_event_track&& data)
        : origin(object_event_track_origin::data)
        , data(std::move(data))
    {
    }

    assets_cell<object_event_track>& fetch(
        manager_assets& objects,
        assets_buffer<object_event_track>& cached_vector,
        const std::filesystem::path& path)
    {
        const std::string _cache_id = path.string();
        return *cached_vector.get_or_create_by_id(_cache_id, [&objects, path] {
            std::shared_ptr<std::promise<object_event_track>> _promise = std::make_shared<std::promise<object_event_track>>();
            objects.fetch_bytes(path, [_promise, path](const std::vector<char>& _bytes) {
				object_event_track _event_track(_bytes);
				_event_track.origin_path = path;
				_promise->set_value(std::move(_event_track)); }, true);

            // create event track on worker thread is ok
            return container_async<object_event_track>(_promise->get_future());
        });
    }

    recipe_object_event_track make_recipe(const assets_cell<object_event_track>& cached)
    {
        const object_event_track& _event_track = cached.fetched.value();

        if (_event_track.origin == object_event_track_origin::path) {
            return recipe_object_event_track_path { cached.fetched.value().origin_path.value() };
        }

        else if (_event_track.origin == object_event_track_origin::data) {
            return recipe_object_event_track_data { _event_track.data };
        }

        else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

    assets_cell<object_event_track>* apply_recipe(manager_assets& objects, assets_buffer<object_event_track>& cached_vector, recipe_object_event_track& recipe)
    {
        return std::visit([&](auto& value) -> assets_cell<object_event_track>* {
            using RecipeType = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<RecipeType, recipe_object_event_track_path>) {
                return &fetch(objects, cached_vector, value.path);

            } else if constexpr (std::is_same_v<RecipeType, recipe_object_event_track_data>) {
                return cached_vector.create_cell(
                    container_async<object_event_track>(
                        object_event_track(std::move(value.data))));

            } else {
                LUCARIA_DEBUG_ERROR("Implementation error");
                return nullptr;
            }
        },
            recipe);
    }

}
}
