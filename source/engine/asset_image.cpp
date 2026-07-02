#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/asset_image.hpp>

namespace lucaria {
namespace detail {

    std::filesystem::path resolve_profile(
        manager_assets& object,
        const std::filesystem::path& path,
        const std::optional<data_image_profile> profile)
    {
        if ((profile == data_image_profile::etc2_rgb4) || (profile == data_image_profile::etc2_rgba8) || object.is_etc2_supported) {
            std::filesystem::path _etc2_path = path;
            _etc2_path = _etc2_path.replace_extension().string() + "_etc.bin";
            if (std::filesystem::exists(object.async_prefix_path / _etc2_path)) {
                return _etc2_path;
            }
        }
        if ((profile == data_image_profile::s3tc_rgb4) || (profile == data_image_profile::s3tc_rgba8) || object.is_s3tc_supported) {
            std::filesystem::path _s3tc_path = path;
            _s3tc_path = _s3tc_path.replace_extension().string() + "_s3tc.bin";
            if (std::filesystem::exists(object.async_prefix_path / _s3tc_path)) {
                return _s3tc_path;
            }
        }
        return path;
    }

    std::array<std::filesystem::path, 6> resolve_profile(
        manager_assets& object,
        const std::array<std::filesystem::path, 6>& paths,
        const std::optional<data_image_profile> profile)
    {
        if ((profile == data_image_profile::etc2_rgb4) || (profile == data_image_profile::etc2_rgba8) || object.is_etc2_supported) {
            std::array<std::filesystem::path, 6> _etc2_paths = paths;
            bool _all_exist = true;
            for (std::size_t _index = 0; _index < 6; ++_index) {
                _etc2_paths[_index] = _etc2_paths[_index].replace_extension().string() + "_etc.bin";
                _all_exist = _all_exist && std::filesystem::exists(object.async_prefix_path / _etc2_paths[_index]);
            }
            if (_all_exist) {
                return _etc2_paths;
            }
        }
        if ((profile == data_image_profile::s3tc_rgb4) || (profile == data_image_profile::s3tc_rgba8) || object.is_s3tc_supported) {
            std::array<std::filesystem::path, 6> _s3tc_paths = paths;
            bool _all_exist = true;
            for (std::size_t _index = 0; _index < 6; ++_index) {
                _s3tc_paths[_index] = _s3tc_paths[_index].replace_extension().string() + "_s3tc.bin";
                _all_exist = _all_exist && std::filesystem::exists(object.async_prefix_path / _s3tc_paths[_index]);
            }
            if (_all_exist) {
                return _s3tc_paths;
            }
        }
        return paths;
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
