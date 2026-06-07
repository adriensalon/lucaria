#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/serialize_context.hpp>
#include <lucaria/engine/asset_animation.hpp>

namespace lucaria {
namespace detail {

    asset_animation::asset_animation(const std::vector<char>& bytes)
    {
        assets_bytes_stream_ozz _ozz_stream(bytes);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
        if (!_ozz_archive.TestTag<ozz::animation::Animation>()) {
            LUCARIA_DEBUG_ERROR("Failed to load animation, archive doesn't contain the expected object type")
        }
        _ozz_archive >> animation;
    }

    void asset_animation::save(storage_save_context& context) const
    {
        context.field("origin_path", origin_path);
    }

    void asset_animation::load(storage_load_context& context)
    {
        context.field("origin_path", origin_path);
        const std::filesystem::path _path = origin_path;
        context.fetch(_path, [this, _path](const std::vector<char>& bytes) {
            *this = asset_animation(bytes);
            origin_path = _path;
        });
    }

}
}
