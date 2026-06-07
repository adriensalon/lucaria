#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/asset_skeleton.hpp>

namespace lucaria {
namespace detail {

    object_skeleton::object_skeleton(const std::vector<char>& bytes)
        : origin(object_skeleton_origin::path)
    {
        assets_bytes_stream_ozz _ozz_stream(bytes);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
        if (!_ozz_archive.TestTag<ozz::animation::Skeleton>()) {
            LUCARIA_DEBUG_ERROR("Failed to load skeleton, archive doesn't contain the expected object type")
        }
        _ozz_archive >> skeleton;
    }

}
}
