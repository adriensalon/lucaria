#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <ozz/animation/runtime/track_sampling_job.h>

#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/object_motion_track.hpp>
#include <lucaria/core/utils_math.hpp>
#include <lucaria/core/utils_stream.hpp>


namespace lucaria {
namespace detail {

    namespace {

        static void _load_motion_track_bytes(ozz::animation::Float3Track& translation_handle, ozz::animation::QuaternionTrack& rotation_handle, const std::vector<char>& bytes)
        {
            ozz_bytes_stream _ozz_stream(bytes);
            ozz::io::IArchive _ozz_archive(&_ozz_stream);
            if (!_ozz_archive.TestTag<ozz::animation::Float3Track>()) {
                LUCARIA_DEBUG_ERROR("Failed to load float3 track, archive doesn't contain the expected object type")
            }
            _ozz_archive >> translation_handle;
            if (!_ozz_archive.TestTag<ozz::animation::QuaternionTrack>()) {
                LUCARIA_DEBUG_ERROR("Impossible to load quaternion track, archive doesn't contain the expected object type")
            }
            _ozz_archive >> rotation_handle;
            // #if defined(LUCARIA_DEBUG)
            //             std::cout << "Loaded motion track with position and rotation" << std::endl;
            // #endif
        }

        static container_async<object_motion_track> _fetch_motion_track_async(manager_object& objects, const std::filesystem::path& path)
        {
            std::shared_ptr<std::promise<object_motion_track>> _promise = std::make_shared<std::promise<object_motion_track>>();
            objects.fetch_bytes(path, [_promise](const std::vector<char>& _bytes) {
				object_motion_track _motion_track(_bytes);
				_promise->set_value(std::move(_motion_track)); }, true);

            // create motion track on worker thread is ok
            return container_async<object_motion_track>(_promise->get_future());
        }

    }

    object_motion_track::object_motion_track(const std::vector<char>& bytes)
        : origin(object_motion_track_origin::path)
    {
        _load_motion_track_bytes(translation_track, rotation_track, bytes);
    }

    object_motion_track::object_motion_track(ozz::animation::Float3Track&& translation_track, ozz::animation::QuaternionTrack&& rotation_track)
        : origin(object_motion_track_origin::path)
        , translation_track(std::move(translation_track))
        , rotation_track(std::move(rotation_track))
    {
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

    container_cache<object_motion_track>& fetch(
        manager_object& objects,
        container_cache_vector<object_motion_track>& cached_vector,
        const std::filesystem::path& path)
    {
        return *cached_vector.get_or_create_by_path(path, [&objects, path] {
            return _fetch_motion_track_async(objects, path);
        });
    }

    recipe_object_motion_track make_recipe(const container_cache<object_motion_track>& cached)
    {
        const object_motion_track& _motion_track = cached.fetched.value();

        if (_motion_track.origin == object_motion_track_origin::path) {
            return recipe_object_motion_track_path { cached.origin_path.value() };
        }

        else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

	container_cache<object_motion_track>* apply_recipe(manager_object& objects, container_cache_vector<object_motion_track>& cached_vector, recipe_object_motion_track& recipe)
    {
        return std::visit([&](auto& value) -> container_cache<object_motion_track>* {
            using RecipeType = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<RecipeType, recipe_object_motion_track_path>) {
                return &fetch(objects, cached_vector, value.path);

            } else {
                LUCARIA_DEBUG_ERROR("Implementation error");
				return nullptr;
            }
        },
            recipe);
    }
}
}
