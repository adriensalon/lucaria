#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <ozz/animation/runtime/track_sampling_job.h>
#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <lucaria/core/track.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/math.hpp>

namespace lucaria {
namespace {

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
    ozz::math::Float3 _position_start;
    ozz::math::Float3 _position_end;

    ozz::animation::Float3TrackSamplingJob _job_start {};
    _job_start.track = &_translation_handle;
    _job_start.ratio = 0.f;
    _job_start.result = &_position_start;
    _job_start.Run();

    ozz::animation::Float3TrackSamplingJob _job_end {};
    _job_end.track = &_translation_handle;
    _job_end.ratio = 1.f;
    _job_end.result = &_position_end;
    _job_end.Run();

    return detail::convert(_position_end - _position_start);
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
