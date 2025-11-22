#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>
#include <ozz/animation/runtime/track_sampling_job.h>

#include <lucaria/core/animation.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/math.hpp>

namespace lucaria {

extern void _load_bytes(const std::filesystem::path& file_path, const std::function<void(const std::vector<char>&)>& callback);
extern void _fetch_bytes(const std::filesystem::path& file_path, const std::function<void(const std::vector<char>&)>& callback, bool persist);

namespace {

    static void load_animation_handle_from_bytes(ozz::animation::Animation& handle, const std::vector<char>& data_bytes)
    {
        _detail::ozz_bytes_stream _ozz_stream(data_bytes);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
        if (!_ozz_archive.TestTag<ozz::animation::Animation>()) {
            LUCARIA_RUNTIME_ERROR("Failed to load animation, archive doesn't contain the expected object type")
        }
        _ozz_archive >> handle;
#if LUCARIA_CONFIG_DEBUG
        std::cout << "Loaded animation with " << handle.num_tracks() << " tracks" << std::endl;
#endif
    }

    static void load_motion_track_handles_from_bytes(ozz::animation::Float3Track& translation_handle, ozz::animation::QuaternionTrack& rotation_handle, const std::vector<char>& data_bytes)
    {
        _detail::ozz_bytes_stream _ozz_stream(data_bytes);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
        if (!_ozz_archive.TestTag<ozz::animation::Float3Track>()) {
            LUCARIA_RUNTIME_ERROR("Failed to load float3 track, archive doesn't contain the expected object type")
        }
        _ozz_archive >> translation_handle;
        if (!_ozz_archive.TestTag<ozz::animation::QuaternionTrack>()) {
            LUCARIA_RUNTIME_ERROR("Impossible to load quaternion track, archive doesn't contain the expected object type")
        }
        _ozz_archive >> rotation_handle;
#if LUCARIA_CONFIG_DEBUG
        std::cout << "Loaded motion track with position and rotation" << std::endl;
#endif
    }

    static void load_event_track_data_from_bytes(event_track_data& data, const std::vector<char>& data_bytes)
    {
        _detail::bytes_stream _stream(data_bytes);
#if LUCARIA_JSON
        cereal::JSONInputArchive _archive(_stream);
#else
        cereal::PortableBinaryInputArchive _archive(_stream);
#endif
        _archive(data);
#if LUCARIA_CONFIG_DEBUG
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
    _load_bytes(data_path, [this](const std::vector<char>& _data_bytes) {
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

animation_motion_track::animation_motion_track(const std::vector<char>& data_bytes)
{
    load_motion_track_handles_from_bytes(_translation_handle, _rotation_handle, data_bytes);
}

animation_motion_track::animation_motion_track(const std::filesystem::path& data_path)
{
    _load_bytes(data_path, [this](const std::vector<char>& _data_bytes) {
        load_motion_track_handles_from_bytes(_translation_handle, _rotation_handle, _data_bytes);
    });
}

ozz::animation::Float3Track& animation_motion_track::get_translation_handle()
{
    return _translation_handle;
}

const ozz::animation::Float3Track& animation_motion_track::get_translation_handle() const
{
    return _translation_handle;
}

ozz::animation::QuaternionTrack& animation_motion_track::get_rotation_handle()
{
    return _rotation_handle;
}

const ozz::animation::QuaternionTrack& animation_motion_track::get_rotation_handle() const
{
    return _rotation_handle;
}

glm::vec3 animation_motion_track::get_total_translation() const
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
    return convert(_position_end - _position_start);
}

animation_event_track::animation_event_track(const std::vector<char>& data_bytes)
{
    load_event_track_data_from_bytes(data, data_bytes);
}

animation_event_track::animation_event_track(const std::filesystem::path& data_path)
{
    _load_bytes(data_path, [this](const std::vector<char>& _data_bytes) {
        load_event_track_data_from_bytes(data, _data_bytes);
    });
}

fetched<animation> fetch_animation(const std::filesystem::path& data_path)
{
    std::shared_ptr<std::promise<animation>> _promise = std::make_shared<std::promise<animation>>();
    _fetch_bytes(data_path, [_promise](const std::vector<char>& _data_bytes) {
        animation _animation(_data_bytes);
        _promise->set_value(std::move(_animation)); }, true);

    // create animation on worker thread is ok
    return fetched<animation>(_promise->get_future());
}

fetched<animation_motion_track> fetch_motion_track(const std::filesystem::path& data_path)
{
    std::shared_ptr<std::promise<animation_motion_track>> _promise = std::make_shared<std::promise<animation_motion_track>>();
    _fetch_bytes(data_path, [_promise](const std::vector<char>& _data_bytes) {
        animation_motion_track _motion_track(_data_bytes);
        _promise->set_value(std::move(_motion_track)); }, true);

    // create motion track on worker thread is ok
    return fetched<animation_motion_track>(_promise->get_future());
}

fetched<animation_event_track> fetch_event_track(const std::filesystem::path& data_path)
{
    std::shared_ptr<std::promise<animation_event_track>> _promise = std::make_shared<std::promise<animation_event_track>>();
    _fetch_bytes(data_path, [_promise](const std::vector<char>& _data_bytes) {
        animation_event_track _event_track(_data_bytes);
        _promise->set_value(std::move(_event_track)); }, true);

    // create event track on worker thread is ok
    return fetched<animation_event_track>(_promise->get_future());
}

}
