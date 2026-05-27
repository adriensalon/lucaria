#include <ozz/animation/runtime/track_sampling_job.h>

#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/object_animation.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static void _load_animation_bytes(ozz::animation::Animation& animation, const std::vector<char>& bytes)
        {
            ozz_bytes_stream _ozz_stream(bytes);
            ozz::io::IArchive _ozz_archive(&_ozz_stream);
            if (!_ozz_archive.TestTag<ozz::animation::Animation>()) {
                LUCARIA_DEBUG_ERROR("Failed to load animation, archive doesn't contain the expected object type")
            }
            _ozz_archive >> animation;
        }

        static container_async<object_animation> _fetch_animation_async(manager_object& objects, const std::filesystem::path& path)
        {
            std::shared_ptr<std::promise<object_animation>> _promise = std::make_shared<std::promise<object_animation>>();
            objects.fetch_bytes(path, [_promise](const std::vector<char>& _bytes) {
				object_animation _animation(_bytes);
				_promise->set_value(std::move(_animation)); }, true);

            // create animation on worker thread is ok
            return container_async<object_animation>(_promise->get_future());
        }

    }

    object_animation::object_animation(const std::vector<char>& bytes)
        : origin(object_animation_origin::path)
    {
        _load_animation_bytes(animation, bytes);
    }

    object_animation::object_animation(ozz::animation::Animation&& animation)
        : origin(object_animation_origin::path)
        , animation(std::move(animation))
    {
    }

    container_cache<object_animation>& fetch(
		manager_object& objects,
        container_cache_vector<object_animation>& cached_vector,
        const std::filesystem::path& path)
    {
        return *cached_vector.get_or_create_by_path(path, [&objects, path] {
            return _fetch_animation_async(objects, path);
        });
    }

    recipe_object_animation make_recipe(const container_cache<object_animation>& cached)
    {
        const object_animation& _animation = cached.fetched.value();

        if (_animation.origin == object_animation_origin::path) {
            return recipe_object_animation_path { cached.origin_path.value() };

        } else {
            LUCARIA_DEBUG_ERROR("Invalid implementation");
            return {};
        }
    }

	container_cache<object_animation>* apply_recipe(manager_object& objects, container_cache_vector<object_animation>& cached_vector, recipe_object_animation& recipe)
    {
        return std::visit([&](auto& value) -> container_cache<object_animation>* {
            using RecipeType = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<RecipeType, recipe_object_animation_path>) {
                return &fetch(objects, cached_vector, value.path);

            } else {
                LUCARIA_DEBUG_ERROR("Implementation error");
				return nullptr;
            }
        },
            recipe);
    }

}
}
