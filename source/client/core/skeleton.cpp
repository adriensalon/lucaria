#include <iostream>

#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <lucaria/core/database.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/skeleton.hpp>
#include <lucaria/core/stream.hpp>


namespace lucaria {
namespace detail {

    namespace {

        static void _load_skeleton_bytes(ozz::animation::Skeleton& handle, const std::vector<char>& data_bytes)
        {
            ozz_bytes_stream _ozz_stream(data_bytes);
            ozz::io::IArchive _ozz_archive(&_ozz_stream);
            if (!_ozz_archive.TestTag<ozz::animation::Skeleton>()) {
                LUCARIA_RUNTIME_ERROR("Failed to load skeleton, archive doesn't contain the expected object type")
            }
            _ozz_archive >> handle;
            // #if defined(LUCARIA_DEBUG)
            //             std::cout << "Loaded skeleton with " << handle.num_joints() << " joints." << std::endl;
            // #endif
        }

        static async_container<skeleton_implementation> _fetch_skeleton_async(const std::filesystem::path& data_path)
        {
            std::shared_ptr<std::promise<skeleton_implementation>> _promise = std::make_shared<std::promise<skeleton_implementation>>();
            fetch_bytes(data_path, [_promise](const std::vector<char>& _bytes) {
        skeleton_implementation _skeleton(_bytes);
        _promise->set_value(std::move(_skeleton)); }, true);

            // create skeleton on worker thread is ok
            return detail::async_container<skeleton_implementation>(_promise->get_future());
        }

    }

    skeleton_implementation::skeleton_implementation(const std::vector<char>& bytes)
        : origin(skeleton_origin::path)
    {
        _load_skeleton_bytes(skeleton, bytes);
    }

    skeleton_implementation::skeleton_implementation(ozz::animation::Skeleton&& skeleton)
        : origin(skeleton_origin::path)
		, skeleton(std::move(skeleton))
    {
    }

    skeleton_recipe make_recipe(const implementation_container<skeleton_implementation>& container)
    {
        const skeleton_implementation& _skeleton = container.fetched.value();

        if (_skeleton.origin == skeleton_origin::path) {
            return skeleton_path_recipe { container.origin_path.value() };
        }

        else {
            LUCARIA_RUNTIME_ERROR("Implementation error");
            return {};
        }
    }

}

skeleton_object skeleton_object::fetch(const std::filesystem::path& path)
{
	skeleton_object _skeleton = {};
    _skeleton._resource = detail::engine_resources().skeletons.get_or_create_by_path(path, [&] {
        return detail::_fetch_skeleton_async(path);
    });
    _skeleton._manager = &detail::engine_resources().skeletons;
    _skeleton._refcount.emplace();
    return _skeleton;
}

bool skeleton_object::has_value() const
{
    return _resource && _resource->fetched.has_value();
}

skeleton_object::operator bool() const
{
    return has_value();
}

}
