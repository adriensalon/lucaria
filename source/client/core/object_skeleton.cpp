#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/object_skeleton.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    object_skeleton::object_skeleton(const std::vector<char>& bytes)
        : origin(object_skeleton_origin::path)
    {
        ozz_bytes_stream _ozz_stream(bytes);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
        if (!_ozz_archive.TestTag<ozz::animation::Skeleton>()) {
            LUCARIA_DEBUG_ERROR("Failed to load skeleton, archive doesn't contain the expected object type")
        }
        _ozz_archive >> skeleton;
    }

    assets_cell<object_skeleton>& fetch(
        manager_assets& objects,
        assets_buffer<object_skeleton>& cached_vector,
        const std::filesystem::path& path)
    {
		const std::string _cache_id = path.string();
        return *cached_vector.get_or_create_by_id(_cache_id, [&objects, path] {
            std::shared_ptr<std::promise<object_skeleton>> _promise = std::make_shared<std::promise<object_skeleton>>();
            objects.fetch_bytes(path, [_promise, path](const std::vector<char>& _bytes) {
				object_skeleton _skeleton(_bytes);
				_skeleton.origin_path = path;
				_promise->set_value(std::move(_skeleton)); }, true);

            // create skeleton on worker thread is ok
            return detail::container_async<object_skeleton>(_promise->get_future());
        });
    }

    recipe_object_skeleton make_recipe(const assets_cell<object_skeleton>& cached)
    {
        const object_skeleton& _skeleton = cached.fetched.value();

        if (_skeleton.origin == object_skeleton_origin::path) {
            return recipe_object_skeleton_path { _skeleton.origin_path.value() };
        }

        else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

    assets_cell<object_skeleton>* apply_recipe(manager_assets& objects, assets_buffer<object_skeleton>& cached_vector, recipe_object_skeleton& recipe)
    {
        return std::visit([&](auto& value) -> assets_cell<object_skeleton>* {
            using RecipeType = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<RecipeType, recipe_object_skeleton_path>) {
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
