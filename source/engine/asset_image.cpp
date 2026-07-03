#include <cereal/archives/portable_binary.hpp>

#include <string>

#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/asset_image.hpp>

namespace lucaria {
namespace detail {

    namespace {

        std::filesystem::path resolve_texture_size(
            const std::filesystem::path& path,
            const uint32 texture_size,
            const std::string& profile)
        {
            std::filesystem::path _path = path;
            _path = _path.replace_extension().string() + "." + std::to_string(texture_size) + profile + ".bin";
            return _path;
        }

        bool all_exist(manager_assets& object, const std::array<std::filesystem::path, 6>& paths)
        {
            bool _all_exist = true;
            for (const std::filesystem::path& _path : paths) {
                _all_exist = _all_exist && std::filesystem::exists(object.resolve_fetch_path(_path));
            }
            return _all_exist;
        }

    }

    std::filesystem::path resolve_profile(
        manager_assets& object,
        const std::filesystem::path& path,
        const std::optional<data_image_profile> profile)
    {
        if ((profile == data_image_profile::etc2_rgb4) || (profile == data_image_profile::etc2_rgba8) || object.is_etc2_supported) {
            std::filesystem::path _etc2_path = resolve_texture_size(path, object.texture_size, ".etc");
            if (std::filesystem::exists(object.resolve_fetch_path(_etc2_path))) {
                return _etc2_path;
            }
        }
        if ((profile == data_image_profile::s3tc_rgb4) || (profile == data_image_profile::s3tc_rgba8) || object.is_s3tc_supported) {
            std::filesystem::path _s3tc_path = resolve_texture_size(path, object.texture_size, ".s3tc");
            if (std::filesystem::exists(object.resolve_fetch_path(_s3tc_path))) {
                return _s3tc_path;
            }
        }
        return resolve_texture_size(path, object.texture_size, "");
    }

    std::array<std::filesystem::path, 6> resolve_profile(
        manager_assets& object,
        const std::array<std::filesystem::path, 6>& paths,
        const std::optional<data_image_profile> profile)
    {
        if ((profile == data_image_profile::etc2_rgb4) || (profile == data_image_profile::etc2_rgba8) || object.is_etc2_supported) {
            std::array<std::filesystem::path, 6> _etc2_paths = paths;
            for (std::size_t _index = 0; _index < 6; ++_index) {
                _etc2_paths[_index] = resolve_texture_size(_etc2_paths[_index], object.texture_size, ".etc");
            }
            if (all_exist(object, _etc2_paths)) {
                return _etc2_paths;
            }
        }
        if ((profile == data_image_profile::s3tc_rgb4) || (profile == data_image_profile::s3tc_rgba8) || object.is_s3tc_supported) {
            std::array<std::filesystem::path, 6> _s3tc_paths = paths;
            for (std::size_t _index = 0; _index < 6; ++_index) {
                _s3tc_paths[_index] = resolve_texture_size(_s3tc_paths[_index], object.texture_size, ".s3tc");
            }
            if (all_exist(object, _s3tc_paths)) {
                return _s3tc_paths;
            }
        }
        std::array<std::filesystem::path, 6> _raw_paths = paths;
        for (std::size_t _index = 0; _index < 6; ++_index) {
            _raw_paths[_index] = resolve_texture_size(_raw_paths[_index], object.texture_size, "");
        }
        return _raw_paths;
    }

    asset_image::asset_image(const std::vector<char>& bytes)
        : origin(object_image_origin::path)
    {
        assets_bytes_stream _stream(bytes);
        cereal::PortableBinaryInputArchive _archive(_stream);
        _archive(data);
        profile = data.profile;
    }

    asset_image::asset_image(data_image&& data)
        : origin(object_image_origin::data)
        , profile(data.profile)
        , data(std::move(data))
    {
    }

}
}
