#include <ozz/animation/runtime/track_sampling_job.h>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/object_animation.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    object_animation::object_animation(const std::vector<char>& bytes)
        : origin(object_animation_origin::path)
    {
        ozz_bytes_stream _ozz_stream(bytes);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
        if (!_ozz_archive.TestTag<ozz::animation::Animation>()) {
            LUCARIA_DEBUG_ERROR("Failed to load animation, archive doesn't contain the expected object type")
        }
        _ozz_archive >> animation;
    }

}
}
