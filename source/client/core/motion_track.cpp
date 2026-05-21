#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <ozz/animation/runtime/track_sampling_job.h>
#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <lucaria/core/database.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/motion_track.hpp>
#include <lucaria/core/stream.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static void _load_motion_track_bytes(ozz::animation::Float3Track& translation_handle, ozz::animation::QuaternionTrack& rotation_handle, const std::vector<char>& bytes)
        {
            ozz_bytes_stream _ozz_stream(bytes);
            ozz::io::IArchive _ozz_archive(&_ozz_stream);
            if (!_ozz_archive.TestTag<ozz::animation::Float3Track>()) {
                LUCARIA_RUNTIME_ERROR("Failed to load float3 track, archive doesn't contain the expected object type")
            }
            _ozz_archive >> translation_handle;
            if (!_ozz_archive.TestTag<ozz::animation::QuaternionTrack>()) {
                LUCARIA_RUNTIME_ERROR("Impossible to load quaternion track, archive doesn't contain the expected object type")
            }
            _ozz_archive >> rotation_handle;
            // #if defined(LUCARIA_DEBUG)
            //             std::cout << "Loaded motion track with position and rotation" << std::endl;
            // #endif
        }

        static async_container<motion_track_implementation> _fetch_motion_track_async(const std::filesystem::path& path)
        {
            std::shared_ptr<std::promise<motion_track_implementation>> _promise = std::make_shared<std::promise<motion_track_implementation>>();
            fetch_bytes(path, [_promise](const std::vector<char>& _bytes) {
        motion_track_implementation _motion_track(_bytes);
        _promise->set_value(std::move(_motion_track)); }, true);

            // create motion track on worker thread is ok
            return async_container<motion_track_implementation>(_promise->get_future());
        }

    }

    motion_track_implementation::motion_track_implementation(const std::vector<char>& bytes)
		: origin(motion_track_origin::path)
    {
        _load_motion_track_bytes(translation_track, rotation_track, bytes);
    }

    motion_track_implementation::motion_track_implementation(ozz::animation::Float3Track&& translation_track, ozz::animation::QuaternionTrack&& rotation_track)
		: origin(motion_track_origin::path)
        , translation_track(std::move(translation_track))
        , rotation_track(std::move(rotation_track))
    {
    }

    glm::vec3 motion_track_implementation::get_total_translation() const
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

    [[nodiscard]] motion_track_recipe make_recipe(const implementation_container<motion_track_implementation>& container)
    {
        const motion_track_implementation& _motion_track = container.fetched.value();

        if (_motion_track.origin == motion_track_origin::path) {
            return motion_track_path_recipe { container.origin_path.value() };
        }

        else {
            LUCARIA_RUNTIME_ERROR("Implementation error");
            return {};
        }
    }
}

motion_track_object motion_track_object::fetch(const std::filesystem::path& path)
{
    detail::implementation_container<detail::motion_track_implementation>* _resource = detail::engine_resources().motion_tracks.get_or_create_by_path(path, [&] {
        return detail::_fetch_motion_track_async(path);
    });

    return motion_track_object { _resource };
}

bool motion_track_object::has_value() const
{
    return _resource && _resource->fetched.has_value();
}

motion_track_object::operator bool() const
{
    return has_value();
}

motion_track_object::motion_track_object(detail::implementation_container<detail::motion_track_implementation>* resource)
    : _resource(resource)
{
}

}
