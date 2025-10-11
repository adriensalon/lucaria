#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>
#include <ozz/animation/runtime/track_sampling_job.h>

#include <lucaria/core/math.hpp>
#include <lucaria/core/animation.hpp>
#include <lucaria/core/error.hpp>

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

    static void load_motion_track_handles_from_bytes(ozz::animation::Float3Track& translation_handle, ozz::animation::QuaternionTrack& rotation_handle, const std::vector<char>& data_bytes)
    {
        detail::ozz_bytes_stream _ozz_stream(data_bytes);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
        if (!_ozz_archive.TestTag<ozz::animation::Float3Track>()) {
            LUCARIA_RUNTIME_ERROR("Failed to load float3 track, archive doesn't contain the expected object type")
        }
        _ozz_archive >> translation_handle;
        if (!_ozz_archive.TestTag<ozz::animation::QuaternionTrack>()) {
            LUCARIA_RUNTIME_ERROR("Impossible to load quaternion track, archive doesn't contain the expected object type")
        }
        _ozz_archive >> rotation_handle;
#if LUCARIA_DEBUG
        std::cout << "Loaded motion track with position and rotation" << std::endl;
#endif
    }

    static void load_event_track_data_from_bytes(event_track_data& data, const std::vector<char>& data_bytes)
    {
        detail::bytes_stream _stream(data_bytes);
#if LUCARIA_JSON
        cereal::JSONInputArchive _archive(_stream);
#else
        cereal::PortableBinaryInputArchive _archive(_stream);
#endif
        _archive(data);
#if LUCARIA_DEBUG
        std::cout << "Loaded event track" << std::endl;
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

motion_track::motion_track(const std::vector<char>& data_bytes)
{
    load_motion_track_handles_from_bytes(_translation_handle, _rotation_handle, data_bytes);
}

motion_track::motion_track(const std::filesystem::path& data_path)
{
    detail::load_bytes(data_path, [this](const std::vector<char>& _data_bytes) {
        load_motion_track_handles_from_bytes(_translation_handle, _rotation_handle, _data_bytes);
    });
}

ozz::animation::Float3Track& motion_track::get_translation_handle()
{
    return _translation_handle;
}

const ozz::animation::Float3Track& motion_track::get_translation_handle() const
{
    return _translation_handle;
}

ozz::animation::QuaternionTrack& motion_track::get_rotation_handle()
{
    return _rotation_handle;
}

const ozz::animation::QuaternionTrack& motion_track::get_rotation_handle() const
{
    return _rotation_handle;
}

glm::vec3 motion_track::get_total_translation() const
{
    ozz::math::Float3 t0_ozz, t1_ozz;

    {
        ozz::animation::Float3TrackSamplingJob job {};
        job.track = &_translation_handle;
        job.ratio = 0.f; // start
        job.result = &t0_ozz;
        job.Run();
    }

    {
        ozz::animation::Float3TrackSamplingJob job {};
        job.track = &_translation_handle;
        job.ratio = 1.f; // end
        job.result = &t1_ozz;
        job.Run();
    }

    return detail::reinterpret(t1_ozz - t0_ozz); 
}

fetched<motion_track> fetch_motion_track(const std::filesystem::path& data_path)
{
    std::shared_ptr<std::promise<motion_track>> _promise = std::make_shared<std::promise<motion_track>>();

    detail::fetch_bytes(data_path, [_promise](const std::vector<char>& _data_bytes) {
        motion_track _motion_track(_data_bytes);
        _promise->set_value(std::move(_motion_track));
    });

    // create motion track on worker thread is ok
    return fetched<motion_track>(_promise->get_future());
}

// event track

event_track::event_track(const std::vector<char>& data_bytes)
{
    load_event_track_data_from_bytes(data, data_bytes);
}

event_track::event_track(const std::filesystem::path& data_path)
{
    detail::load_bytes(data_path, [this](const std::vector<char>& _data_bytes) {
        load_event_track_data_from_bytes(data, _data_bytes);
    });
}

fetched<event_track> fetch_event_track(const std::filesystem::path& data_path)
{
    std::shared_ptr<std::promise<event_track>> _promise = std::make_shared<std::promise<event_track>>();

    detail::fetch_bytes(data_path, [_promise](const std::vector<char>& _data_bytes) {
        event_track _event_track(_data_bytes);
        _promise->set_value(std::move(_event_track));
    });

    // create event track on worker thread is ok
    return fetched<event_track>(_promise->get_future());
}

}
