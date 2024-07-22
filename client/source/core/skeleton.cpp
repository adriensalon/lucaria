#include <iostream>

#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <core/skeleton.hpp>
#include <core/fetch.hpp>
#include <core/load.hpp>

namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<skeleton_ref>>> promises;

}

std::shared_future<std::shared_ptr<skeleton_ref>> fetch_skeleton(const std::filesystem::path& skeleton_path)
{
    std::promise<std::shared_ptr<skeleton_ref>>& _promise = detail::promises[skeleton_path.string()];
    fetch_file(skeleton_path, [&_promise](std::istringstream& stream) {
        std::shared_ptr<skeleton_ref> _skeleton = std::make_shared<skeleton_ref>();
        ozz::io::StdStringStreamWrapper _ozz_stream(stream);
        {
            ozz::io::IArchive _ozz_archive(&_ozz_stream);
#if LUCARIA_DEBUG
            if (!_ozz_archive.TestTag<ozz::animation::Skeleton>()) {
                std::cout << "Impossible to load skeleton, archive doesn't contain the expected object type." << std::endl;
                std::terminate();
            }
#endif
            _ozz_archive >> *(_skeleton.get());
#if LUCARIA_DEBUG
            std::cout << "Loaded skeleton with " << _skeleton->num_joints() << " joints." << std::endl;
#endif
        }
        _promise.set_value(_skeleton);
    });
    return _promise.get_future();
}
