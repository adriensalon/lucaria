#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/cubemap.hpp>
#include <lucaria/core/database.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/fetch.hpp>

namespace lucaria {

extern const std::filesystem::path& _resolve_image_path(const std::filesystem::path& data_path, const std::optional<std::filesystem::path>& etc2_path, const std::optional<std::filesystem::path>& s3tc_path);
extern std::vector<std::filesystem::path> _resolve_image_paths(const std::array<std::filesystem::path, 6>& data_paths, const std::optional<std::array<std::filesystem::path, 6>>& etc2_paths, const std::optional<std::array<std::filesystem::path, 6>>& s3tc_paths);

namespace detail {

    namespace {

        static async_container<cubemap_implementation> _fetch_cubemap_async(
            const std::array<std::filesystem::path, 6>& data_paths,
            const std::optional<std::array<std::filesystem::path, 6>>& etc2_paths,
            const std::optional<std::array<std::filesystem::path, 6>>& s3tc_paths)
        {
            const std::vector<std::filesystem::path> _image_paths = _resolve_image_paths(data_paths, etc2_paths, s3tc_paths);
            std::shared_ptr<std::promise<std::array<image_implementation, 6>>> _images_promise = std::make_shared<std::promise<std::array<image_implementation, 6>>>();
            fetch_bytes(_image_paths, [_images_promise](const std::vector<std::vector<char>>& _data_bytes) {
        std::array<image_implementation, 6> _images = {
            image_implementation(_data_bytes[0]),
            image_implementation(_data_bytes[1]),
            image_implementation(_data_bytes[2]),
            image_implementation(_data_bytes[3]),
            image_implementation(_data_bytes[4]),
            image_implementation(_data_bytes[5])
        };
        _images_promise->set_value(std::move(_images)); }, true);

            // create cubemap on main thread
            return async_container<cubemap_implementation>(_images_promise->get_future(), [](const std::array<image_implementation, 6>& _images) {
                return cubemap_implementation(_images);
            });
        }

    }

    cubemap_recipe make_recipe(const implementation_container<cubemap_implementation>& container)
    {
        const cubemap_implementation& _cubemap = container.fetched.value();

        if (_cubemap.origin == cubemap_origin::path) {
            // return cubemap_path_recipe { _resolve_image_paths(
            //     std::array<std::filesystem::path, 6> {
            //         container.origin_path.value() },
            //     std::nullopt,
            //     std::nullopt) };
			return {};
            // TODO we should be able to serialize etc2 and s3tc paths if they exist, but for now we just put nullopt
        }

        else if (_cubemap.origin == cubemap_origin::data) {
            // return cubemap_data_recipe { _cubemap.data };
			// TODO
			return {};
        }

        else {
            LUCARIA_RUNTIME_ERROR("Implementation error");
            return {};
        }
    }
}

cubemap_object::~cubemap_object()
{
    if (_refcount.is_last_owner()) {
        _manager->destroy_cell(_resource);
    }
}

cubemap_object cubemap_object::fetch(
    const std::array<std::filesystem::path, 6>& data_paths,
    const std::optional<std::array<std::filesystem::path, 6>>& etc2_paths,
    const std::optional<std::array<std::filesystem::path, 6>>& s3tc_paths)
{
	cubemap_object _cubemap = {};
    _cubemap._resource = detail::engine_resources().cubemaps.get_or_create_by_path(data_paths[0], [&] {
        return detail::_fetch_cubemap_async(data_paths, etc2_paths, s3tc_paths);
    });
    _cubemap._manager = &detail::engine_resources().cubemaps;
    _cubemap._refcount.emplace();
    return _cubemap;
}

bool cubemap_object::has_value() const
{
    return _resource && _resource->fetched.has_value();
}

cubemap_object::operator bool() const
{
    return has_value();
}

}
