#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/object_cubemap.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static container_async<object_cubemap> _fetch_cubemap_async(
            manager_object& objects,
            const std::array<std::filesystem::path, 6>& paths)
        {
            std::shared_ptr<std::promise<std::array<object_image, 6>>> _images_promise = std::make_shared<std::promise<std::array<object_image, 6>>>();
            objects.fetch_bytes(paths, [_images_promise](const std::vector<std::vector<char>>& _bytes) {
				std::array<object_image, 6> _images = {
					object_image(_bytes[0]),
					object_image(_bytes[1]),
					object_image(_bytes[2]),
					object_image(_bytes[3]),
					object_image(_bytes[4]),
					object_image(_bytes[5])
				};
				_images_promise->set_value(std::move(_images)); }, true);

            // create cubemap on main thread
            return container_async<object_cubemap>(_images_promise->get_future(), [](const std::array<object_image, 6>& _images) {
                return object_cubemap(_images);
            });
        }

    }

    container_cache<object_cubemap>& fetch(
        manager_object& objects,
        container_cache_vector<object_cubemap>& cached_vector,
        const std::array<std::filesystem::path, 6>& paths,
        const std::optional<data_image_profile> profile)
    {
        const std::array<std::filesystem::path, 6> _resolved_paths = resolve_profile(objects, paths, profile);
        return *cached_vector.get_or_create_by_path(_resolved_paths[0], [&objects, _resolved_paths] {
            return _fetch_cubemap_async(objects, _resolved_paths);
        });
    }

    recipe_object_cubemap make_recipe(const container_cache<object_cubemap>& container)
    {
        const object_cubemap& _cubemap = container.fetched.value();

        if (_cubemap.origin == object_cubemap_origin::path) {
            // return recipe_object_cubemap_path {} // TODO W PROFILE ?
            return {};
        }

        else if (_cubemap.origin == object_cubemap_origin::data) {
            // return recipe_object_cubemap_data { _cubemap.data }; TODO
            return {};
        }

        else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

    container_cache<object_cubemap>* apply_recipe(manager_object& objects, container_cache_vector<object_cubemap>& cached_vector, recipe_object_cubemap& recipe)
    {
        return std::visit([&](auto& value) -> container_cache<object_cubemap>* {
            using RecipeType = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<RecipeType, recipe_object_cubemap_path>) {
                return &fetch(objects, cached_vector, value.paths, value.profile);

            } else if constexpr (std::is_same_v<RecipeType, recipe_object_cubemap_data>) {
                return cached_vector.create_cell(
                    container_async<object_cubemap>(
                        object_cubemap({
							
                            object_image(std::move(value.datas[0])),
                            object_image(std::move(value.datas[1])),
                            object_image(std::move(value.datas[2])),
                            object_image(std::move(value.datas[3])),
                            object_image(std::move(value.datas[4])),
                            object_image(std::move(value.datas[5])) })));

            } else {
                LUCARIA_DEBUG_ERROR("Implementation error");
                return nullptr;
            }
        },
            recipe);
    }
}
}
