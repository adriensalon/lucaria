#include <iostream>

#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <external/ozz/io/stringstream.hpp>
#include <core/skeleton.hpp>
#include <glue/fetch.hpp>

namespace detail {

static std::unordered_map<std::string, std::promise<skeleton_ref>> promises;

}

ozz::animation::Skeleton& skeleton_ref::get_skeleton()
{
    return _skeleton;
}

skeleton_ref load_skeleton(const std::filesystem::path& file)
{
    ozz::io::File _ozz_file(file.c_str(), "rb");
#if LUCARIA_DEBUG
    if (!_ozz_file.opened()) {
        std::cout << "Impossible to open file '" << file << "'." << std::endl;
        std::terminate();
    }
#endif
    ozz::io::IArchive _ozz_archive(&_ozz_file);
#if LUCARIA_DEBUG
    if (!_ozz_archive.TestTag<ozz::animation::Skeleton>()) {
        std::cout << "Impossible to load skeleton, archive doesn't contain the expected object type." << std::endl;
        std::terminate();
    }
#endif
    skeleton_ref _ref;
    _ozz_archive >> _ref._skeleton;
#if LUCARIA_DEBUG
    std::cout << "Loaded animation data from " << file << std::endl;
#endif
    return _ref;
}

std::future<skeleton_ref> fetch_skeleton(const std::filesystem::path& file)
{
    const std::string _file_str = file.string();
    std::promise<skeleton_ref>& _promise = detail::promises[_file_str];
    fetch_file(_file_str, [&_promise, file](std::istringstream& stream) {
        ozz::io::StdStringStreamWrapper _ozz_stream(stream);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
#if LUCARIA_DEBUG
        if (!_ozz_archive.TestTag<ozz::animation::Skeleton>()) {
            std::cout << "Impossible to load skeleton, archive doesn't contain the expected object type." << std::endl;
            std::terminate();
        }
#endif
        skeleton_ref _ref;
        _ozz_archive >> _ref._skeleton;
#if LUCARIA_DEBUG
        std::cout << "Loaded skeleton data from " << file << std::endl;
#endif
        _promise.set_value(std::move(_ref));
    });
    return _promise.get_future();
}
