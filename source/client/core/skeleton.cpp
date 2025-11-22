#include <iostream>

#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <lucaria/core/error.hpp>
#include <lucaria/core/skeleton.hpp>

namespace lucaria {
    
extern void _load_bytes(const std::filesystem::path& file_path, const std::function<void(const std::vector<char>&)>& callback);
extern void _fetch_bytes(const std::filesystem::path& file_path, const std::function<void(const std::vector<char>&)>& callback, bool persist);

namespace {

    static void load_skeleton_handle_from_bytes(ozz::animation::Skeleton& handle, const std::vector<char>& data_bytes)
    {
        _detail::ozz_bytes_stream _ozz_stream(data_bytes);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
        if (!_ozz_archive.TestTag<ozz::animation::Skeleton>()) {
            LUCARIA_RUNTIME_ERROR("Failed to load skeleton, archive doesn't contain the expected object type")
        }
        _ozz_archive >> handle;
#if LUCARIA_CONFIG_DEBUG
        std::cout << "Loaded skeleton with " << handle.num_joints() << " joints." << std::endl;
#endif
    }

}

skeleton::skeleton(const std::vector<char>& data_bytes)
{
    load_skeleton_handle_from_bytes(_handle, data_bytes);
}

skeleton::skeleton(const std::filesystem::path& data_path)
{
    _load_bytes(data_path, [this](const std::vector<char>& _data_bytes) {
        load_skeleton_handle_from_bytes(_handle, _data_bytes);
    });
}

ozz::animation::Skeleton& skeleton::get_handle()
{
    return _handle;
}

fetched<skeleton> fetch_skeleton(const std::filesystem::path& data_path)
{
    std::shared_ptr<std::promise<skeleton>> _promise = std::make_shared<std::promise<skeleton>>();
    _fetch_bytes(data_path, [_promise](const std::vector<char>& _data_bytes) {
        skeleton _skeleton(_data_bytes);
        _promise->set_value(std::move(_skeleton));
    }, true);

    // create skeleton on worker thread is ok
    return fetched<skeleton>(_promise->get_future());
}

}
