#include <lucaria/core/database.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/texture.hpp>


namespace lucaria {

extern const std::filesystem::path& _resolve_image_path(const std::filesystem::path& data_path, const std::optional<std::filesystem::path>& etc2_path, const std::optional<std::filesystem::path>& s3tc_path);

namespace detail {

    namespace {

        static async_container<texture_implementation> _fetch_texture_async(
            const std::filesystem::path& path,
            const std::optional<std::filesystem::path>& etc2_path,
            const std::optional<std::filesystem::path>& s3tc_path)
        {
            const std::filesystem::path& _image_path = _resolve_image_path(path, etc2_path, s3tc_path);
            std::shared_ptr<std::promise<image_implementation>> _image_promise = std::make_shared<std::promise<image_implementation>>();
            fetch_bytes(_image_path, [_image_promise](const std::vector<char>& _data_bytes) {
        image_implementation _image(_data_bytes);
        _image_promise->set_value(std::move(_image)); }, true);

            // create texture on main thread
            return async_container<texture_implementation>(_image_promise->get_future(), [](const image_implementation& _from) {
                return texture_implementation(_from);
            });
        }

    }

    texture_recipe make_recipe(const implementation_container<texture_implementation>& container)
    {
        const texture_implementation& _texture = container.fetched.value();

        if (_texture.origin == texture_origin::path) {
            return texture_path_recipe { container.origin_path.value() };
        }

        // else if (_texture.origin == texture_origin::data) {
        //     return texture_data_recipe { image_implementation(_texture).data };
        // }

        else if (_texture.origin == texture_origin::size) {
            return texture_size_recipe { _texture.size };
        }

        else {
            LUCARIA_RUNTIME_ERROR("Implementation error");
            return {};
        }
    }
}

texture_object::~texture_object()
{
    if (_refcount.is_last_owner()) {
        _manager->destroy_cell(_resource);
    }
}

texture_object texture_object::create(const uint32x2 size)
{
    return texture_object { detail::engine_resources().textures.create_cell(
        detail::async_container<detail::texture_implementation>(
            detail::texture_implementation(size))) };
}

texture_object texture_object::fetch(const std::filesystem::path& path, const std::optional<std::filesystem::path>& etc2_path, const std::optional<std::filesystem::path>& s3tc_path)
{
    detail::implementation_container<detail::texture_implementation>* _resource = detail::engine_resources().textures.get_or_create_by_path(path, [&] {
        return detail::_fetch_texture_async(path, etc2_path, s3tc_path);
    });

    return texture_object { _resource };
}

bool texture_object::has_value() const
{
    return _resource && _resource->fetched.has_value();
}

texture_object::operator bool() const
{
    return has_value();
}

void texture_object::resize(const uint32x2 new_size)
{
    if (has_value()) {
        _resource->fetched.value().resize(new_size);
    }
}

uint32x2 texture_object::size() const
{
    if (has_value()) {
        return _resource->fetched.value().size;
    }

    return uint32x2(0, 0);
}

ImTextureID texture_object::imgui_texture() const
{
    if (has_value()) {
        return _resource->fetched.value().imgui_texture();
    }

    return static_cast<ImTextureID>(0);
}

texture_object::texture_object(detail::implementation_container<detail::texture_implementation>* resource)
    : _resource(resource)
{
}

}
