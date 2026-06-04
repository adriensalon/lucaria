#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <ozz/animation/runtime/track_sampling_job.h>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/object_motion_track.hpp>
#include <lucaria/core/utils_math.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    object_motion_track::object_motion_track(const std::vector<char>& bytes)
        : origin(object_motion_track_origin::path)
    {
        ozz_bytes_stream _ozz_stream(bytes);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
        if (!_ozz_archive.TestTag<ozz::animation::Float3Track>()) {
            LUCARIA_DEBUG_ERROR("Failed to load float3 track, archive doesn't contain the expected object type")
        }
        _ozz_archive >> translation_track;
        if (!_ozz_archive.TestTag<ozz::animation::QuaternionTrack>()) {
            LUCARIA_DEBUG_ERROR("Impossible to load quaternion track, archive doesn't contain the expected object type")
        }
        _ozz_archive >> rotation_track;
    }

    float32x3 object_motion_track::get_total_translation() const
    {
        ozz::math::Float3 _position_start;
        ozz::math::Float3 _position_end;
        ozz::animation::Float3TrackSamplingJob _job_start {};
        _job_start.track = &translation_track;
        _job_start.ratio = 0.f;
        _job_start.result = &_position_start;
        _job_start.Run();
        ozz::animation::Float3TrackSamplingJob _job_end {};
        _job_end.track = &translation_track;
        _job_end.ratio = 1.f;
        _job_end.result = &_position_end;
        _job_end.Run();
        return convert(_position_end - _position_start);
    }

}
}
