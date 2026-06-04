#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/object_image.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    std::filesystem::path resolve_profile(
        manager_assets& object,
        const std::filesystem::path& path,
        const std::optional<data_image_profile> profile)
    {
        if ((!profile || profile == data_image_profile::etc2_compressed) && object.is_etc2_supported) {
            std::filesystem::path _etc2_path = path;
            _etc2_path = _etc2_path.replace_extension().string() + "_etc.bin";
            if (!profile && std::filesystem::exists(object.async_prefix_path / _etc2_path)) {
                return _etc2_path;
            }
            if (profile == data_image_profile::etc2_compressed) {
                LUCARIA_DEBUG_ASSERT(std::filesystem::exists(object.async_prefix_path / _etc2_path), "ETC2 path does not exist");
                return _etc2_path;
            }
        }

        if ((!profile || profile == data_image_profile::s3tc_compressed) && object.is_s3tc_supported) {
            std::filesystem::path _s3tc_path = path;
            _s3tc_path = _s3tc_path.replace_extension().string() + "_s3tc.bin";
            if (!profile && std::filesystem::exists(object.async_prefix_path / _s3tc_path)) {
                return _s3tc_path;
            }
            if (profile == data_image_profile::s3tc_compressed) {
                LUCARIA_DEBUG_ASSERT(std::filesystem::exists(object.async_prefix_path / _s3tc_path), "S3TC path does not exist");
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
        if ((!profile || profile == data_image_profile::etc2_compressed) && object.is_etc2_supported) {
            std::array<std::filesystem::path, 6> _etc2_paths = paths;
            bool _all_exist = true;
            for (std::size_t _index = 0; _index < 6; ++_index) {
                _etc2_paths[_index] = _etc2_paths[_index].replace_extension().string() + "_etc.bin";
                _all_exist = _all_exist && std::filesystem::exists(object.async_prefix_path / _etc2_paths[_index]);
            }
            if (!profile && _all_exist) {
                return _etc2_paths;
            }
            if (profile == data_image_profile::etc2_compressed) {
                for (std::size_t _index = 0; _index < 6; ++_index) {
                    LUCARIA_DEBUG_ASSERT(std::filesystem::exists(object.async_prefix_path / _etc2_paths[_index]), "ETC2 path does not exist");
                }
                return _etc2_paths;
            }
        }

        if ((!profile || profile == data_image_profile::s3tc_compressed) && object.is_s3tc_supported) {
            std::array<std::filesystem::path, 6> _s3tc_paths = paths;
            bool _all_exist = true;
            for (std::size_t _index = 0; _index < 6; ++_index) {
                _s3tc_paths[_index] = _s3tc_paths[_index].replace_extension().string() + "_s3tc.bin";
                _all_exist = _all_exist && std::filesystem::exists(object.async_prefix_path / _s3tc_paths[_index]);
            }
            if (!profile && _all_exist) {
                return _s3tc_paths;
            }
            if (profile == data_image_profile::s3tc_compressed) {
                for (std::size_t _index = 0; _index < 6; ++_index) {
                    LUCARIA_DEBUG_ASSERT(std::filesystem::exists(object.async_prefix_path / _s3tc_paths[_index]), "S3TC path does not exist");
                }
                return _s3tc_paths;
            }
        }

        return paths;
    }

    object_image::object_image(const std::vector<char>& bytes)
        : origin(object_image_origin::path)
    {
        const std::vector<uint8_t>& _content = *(reinterpret_cast<const std::vector<uint8_t>*>(&bytes));
        const uint32_t* _data32 = (uint32_t*)_content.data();
        data.profile = data_image_profile::binary;

        // PVR
        if (*_data32 == 0x03525650) {
            switch (*(_data32 + 2)) {
            case 7:
                // std::cout << "PVR DXT1 RGB" << std::endl;
                data.channels = 3;
                data.profile = data_image_profile::s3tc_compressed;
                break;
            case 11:
                // std::cout << "PVR DXT5 RGBA" << std::endl;
                data.channels = 4;
                data.profile = data_image_profile::s3tc_compressed;
                break;
            case 22:
                // std::cout << "PVR ETC2 RGB" << std::endl;
                data.channels = 3;
                data.profile = data_image_profile::etc2_compressed;
                break;
            case 23:
                // std::cout << "PVR ETC2 RGBA" << std::endl;
                data.channels = 4;
                data.profile = data_image_profile::etc2_compressed;
                break;
            default:
                LUCARIA_DEBUG_ERROR("Invalid S3TC image data")
                break;
            }
            const std::size_t _offset = 52 + *(_data32 + 12);
            const glm::uint8* _data_ptr = _content.data() + _offset;
            const std::size_t _data_size = _content.size() - _offset;
            data.pixels = std::vector<glm::uint8>(_data_ptr, _data_ptr + _data_size);
            data.height = *(_data32 + 6);
            data.width = *(_data32 + 7);

            // KTX
        } else if (*_data32 == 0x58544BAB) {
            switch (*(_data32 + 7)) {
            case 0x9274:
                // std::cout << "KTX ETC2 RGB" << std::endl;
                data.channels = 3;
                data.profile = data_image_profile::etc2_compressed;
                break;
            case 0x9278:
                // std::cout << "KTX ETC2 RGBA" << std::endl;
                data.channels = 4;
                data.profile = data_image_profile::etc2_compressed;
                break;
            default:
                LUCARIA_DEBUG_ERROR("Invalid ETC2 image data")
                break;
            }

            const std::uint8_t* _bytes = reinterpret_cast<const std::uint8_t*>(_content.data());
            const std::uint32_t _header_size = sizeof(std::uint32_t) * 16;
            const std::uint32_t _bytes_of_key_value = *(_data32 + 15);
            std::size_t _offset = _header_size + _bytes_of_key_value;
            const std::uint32_t _image_size = *reinterpret_cast<const std::uint32_t*>(_bytes + _offset);
            _offset += sizeof(std::uint32_t);
            const glm::uint8* _data_ptr = _bytes + _offset;
            const std::size_t _data_size = _image_size;
            data.pixels = std::vector<glm::uint8>(_data_ptr, _data_ptr + _data_size);
            data.width = *(_data32 + 9);
            data.height = *(_data32 + 10);

            // binary raw format
        } else {
            bytes_stream _stream(bytes);
            cereal::PortableBinaryInputArchive _archive(_stream);
            _archive(data);
        }
    }

    object_image::object_image(data_image&& data)
        : origin(object_image_origin::data)
        , profile(data.profile)
        , data(std::move(data))
    {
    }

}
}
