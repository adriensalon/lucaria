#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <ozz/animation/runtime/track_sampling_job.h>
#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <lucaria/core/animation.hpp>
#include <lucaria/core/database.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/stream.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static void _load_animation_bytes(ozz::animation::Animation& handle, const std::vector<char>& bytes)
        {
            ozz_bytes_stream _ozz_stream(bytes);
            ozz::io::IArchive _ozz_archive(&_ozz_stream);
            if (!_ozz_archive.TestTag<ozz::animation::Animation>()) {
                LUCARIA_RUNTIME_ERROR("Failed to load animation, archive doesn't contain the expected object type")
            }
            _ozz_archive >> handle;
            // #if defined(LUCARIA_DEBUG)
            //             std::cout << "Loaded animation with " << handle.num_tracks() << " tracks" << std::endl;
            // #endif
        }

        static async_container<animation_implementation> _fetch_animation_async(const std::filesystem::path& path)
        {
            std::shared_ptr<std::promise<animation_implementation>> _promise = std::make_shared<std::promise<animation_implementation>>();
            fetch_bytes(path, [_promise](const std::vector<char>& _bytes) {
        animation_implementation _animation(_bytes);
        _promise->set_value(std::move(_animation)); }, true);

            // create animation on worker thread is ok
            return async_container<animation_implementation>(_promise->get_future());
        }

    }

    animation_implementation::animation_implementation(const std::vector<char>& bytes)
        : origin(animation_origin::path)
    {
        _load_animation_bytes(animation, bytes);
    }

    animation_implementation::animation_implementation(ozz::animation::Animation&& animation)
        : origin(animation_origin::path)
        , animation(std::move(animation))
    {
    }

    animation_recipe make_recipe(const implementation_container<animation_implementation>& container)
    {
        const animation_implementation& _animation = container.fetched.value();

        if (_animation.origin == animation_origin::path) {
            return animation_path_recipe { container.origin_path.value() };

        } else {
            LUCARIA_RUNTIME_ERROR("Invalid implementation");
            return {};
        }
    }

}

animation_object::~animation_object()
{
    if (_refcount.is_last_owner()) {
        _manager->destroy_cell(_resource);
    }
}

animation_object animation_object::fetch(const std::filesystem::path& path)
{
    detail::implementation_container<detail::animation_implementation>* _resource = detail::engine_resources().animations.get_or_create_by_path(path, [&] {
        return detail::_fetch_animation_async(path);
    });

    return animation_object { _resource };
}

bool animation_object::has_value() const
{
    return _resource && _resource->fetched.has_value();
}

animation_object::operator bool() const
{
    return has_value();
}

animation_object::animation_object(detail::implementation_container<detail::animation_implementation>* resource)
    : _resource(resource)
{
}

}
