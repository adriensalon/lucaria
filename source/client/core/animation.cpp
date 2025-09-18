#include <iostream>

#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <lucaria/core/animation.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/load.hpp>

namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<animation_ref>>> animation_promises;
static std::unordered_map<std::string, std::promise<std::shared_ptr<motion_track_ref>>> motion_track_promises;

}

std::shared_future<std::shared_ptr<animation_ref>> fetch_animation(const std::filesystem::path& animation_path)
{
    std::promise<std::shared_ptr<animation_ref>>& _promise = detail::animation_promises[animation_path.string()];
    fetch_file(animation_path, [&_promise](const std::vector<char>& animation_bytes) {
        std::shared_ptr<animation_ref> _animation = std::make_shared<animation_ref>();
        ozz_raw_input_stream _ozz_stream(animation_bytes);
        {            
            ozz::io::IArchive _ozz_archive(&_ozz_stream);
#if LUCARIA_DEBUG
            if (!_ozz_archive.TestTag<ozz::animation::Animation>()) {
                std::cout << "Impossible to load animation, archive doesn't contain the expected object type." << std::endl;
                std::terminate();
            }
#endif
            _ozz_archive >> *(_animation.get());
#if LUCARIA_DEBUG
            std::cout << "Loaded animation with " << _animation->num_tracks() << " tracks." << std::endl;
#endif
        }
        _promise.set_value(_animation);
    });
    return _promise.get_future();
}

std::shared_future<std::shared_ptr<motion_track_ref>> fetch_motion_track(const std::filesystem::path& motion_track_path)
{
    std::promise<std::shared_ptr<motion_track_ref>>& _promise = detail::motion_track_promises[motion_track_path.string()];
    fetch_file(motion_track_path, [&_promise](const std::vector<char>& motion_track_bytes) {
        std::shared_ptr<motion_track_ref> _motion_track = std::make_shared<motion_track_ref>();
        ozz_raw_input_stream _ozz_stream(motion_track_bytes);
        {
            ozz::io::IArchive _ozz_archive(&_ozz_stream);
#if LUCARIA_DEBUG
            if (!_ozz_archive.TestTag<ozz::animation::Float3Track>()) {
                std::cout << "Impossible to load Float3Track, archive doesn't contain the expected object type." << std::endl;
                std::terminate();
            }
#endif
            _ozz_archive >> _motion_track->first;
#if LUCARIA_DEBUG
            if (!_ozz_archive.TestTag<ozz::animation::QuaternionTrack>()) {
                std::cout << "Impossible to load QuaternionTrack, archive doesn't contain the expected object type." << std::endl;
                std::terminate();
            }
#endif
            _ozz_archive >> _motion_track->second;
#if LUCARIA_DEBUG
            std::cout << "Loaded motion track with position, rotation." << std::endl;
#endif
        }
        _promise.set_value(_motion_track);
    });
    return _promise.get_future();
}

void mark_animation_fetched(const std::filesystem::path& animation_path)
{
    detail::animation_promises.erase(animation_path.string());
}

void mark_motion_track_fetched(const std::filesystem::path& motion_track_path)
{
    detail::motion_track_promises.erase(motion_track_path.string());
}

void clear_animation_fetches()
{
    detail::animation_promises.clear();
    detail::motion_track_promises.clear();
}