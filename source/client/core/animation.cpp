#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <lucaria/core/animation.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/math.hpp>

namespace lucaria {
namespace {

    static void load_animation_handle_from_bytes(ozz::animation::Animation& handle, const std::vector<char>& data_bytes)
    {
        detail::ozz_bytes_stream _ozz_stream(data_bytes);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
        if (!_ozz_archive.TestTag<ozz::animation::Animation>()) {
            LUCARIA_RUNTIME_ERROR("Failed to load animation, archive doesn't contain the expected object type")
        }
        _ozz_archive >> handle;
#if LUCARIA_DEBUG
        std::cout << "Loaded animation with " << handle.num_tracks() << " tracks" << std::endl;
#endif
    }
    
}

animation::animation(const std::vector<char>& data_bytes)
{
    load_animation_handle_from_bytes(_handle, data_bytes);
}

animation::animation(const std::filesystem::path& data_path)
{
    detail::load_bytes(data_path, [this](const std::vector<char>& _data_bytes) {
        load_animation_handle_from_bytes(_handle, _data_bytes);
    });
}

ozz::animation::Animation& animation::get_handle()
{
    return _handle;
}

const ozz::animation::Animation& animation::get_handle() const
{
    return _handle;
}

fetched<animation> fetch_animation(const std::filesystem::path& data_path)
{
    std::shared_ptr<std::promise<animation>> _promise = std::make_shared<std::promise<animation>>();
    detail::fetch_bytes(data_path, [_promise](const std::vector<char>& _data_bytes) {
        animation _animation(_data_bytes);
        _promise->set_value(std::move(_animation));
    });

    // create animation on worker thread is ok
    return fetched<animation>(_promise->get_future());
}

}
