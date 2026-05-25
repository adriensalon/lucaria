#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/object_texture.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static container_async<object_texture> _fetch_texture_async(
            manager_object& objects,
            const std::filesystem::path& path)
        {
            std::shared_ptr<std::promise<object_image>> _image_promise = std::make_shared<std::promise<object_image>>();
            objects.fetch_bytes(path, [_image_promise](const std::vector<char>& _bytes) {
				object_image _image(_bytes);
				_image_promise->set_value(std::move(_image)); }, true);

            // create texture on main thread
            return container_async<object_texture>(_image_promise->get_future(), [](const object_image& _from) {
                return object_texture(_from);
            });
        }

    }

    container_cache<object_texture>& fetch(
        manager_object& objects,
        container_cache_vector<object_texture>& cached_vector,
        const std::filesystem::path& path,
        const std::optional<data_image_profile> profile)
    {
        const std::filesystem::path _resolved_path = resolve_profile(objects, path, profile);
        return *cached_vector.get_or_create_by_path(_resolved_path, [&objects, _resolved_path] {
            return _fetch_texture_async(objects, _resolved_path);
        });
    }

    recipe_object_texture make_recipe(const container_cache<object_texture>& cached)
    {
        const object_texture& _texture = cached.fetched.value();

        if (_texture.origin == object_texture_origin::path) {
            return recipe_object_texture_path { cached.origin_path.value() };
        }

        // else if (_texture.origin == object_texture_origin::data) {
        //     return recipe_object_texture_data { object_image(_texture).data };
        // }

        else if (_texture.origin == object_texture_origin::size) {
            return recipe_object_texture_size { _texture.size };
        }

        else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }
}
}
