#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/image.hpp>

namespace lucaria {

const std::filesystem::path& _resolve_image_path(const std::filesystem::path& data_path, const std::optional<std::filesystem::path>& etc2_path, const std::optional<std::filesystem::path>& s3tc_path);
std::vector<std::filesystem::path> _resolve_image_paths(const std::array<std::filesystem::path, 6>& data_paths, const std::optional<std::array<std::filesystem::path, 6>>& etc2_paths, const std::optional<std::array<std::filesystem::path, 6>>& s3tc_paths);

extern bool _is_etc2_supported;
extern bool _is_s3tc_supported;
extern void _load_bytes(const std::filesystem::path& file_path, const std::function<void(const std::vector<char>&)>& callback);
extern void _fetch_bytes(const std::filesystem::path& file_path, const std::function<void(const std::vector<char>&)>& callback, bool persist);

namespace {

    static void load_data_from_bytes(image_data& data, const std::vector<char>& data_bytes)
    {
        const std::vector<uint8_t>& _content = *(reinterpret_cast<const std::vector<uint8_t>*>(&data_bytes));
        const uint32_t* _data32 = (uint32_t*)_content.data();
        data.is_compressed_etc = false;
        data.is_compressed_s3tc = false;

        // S3TC compression
        if (*_data32 == 0x03525650) {
            switch (*(_data32 + 2)) {
            case 7:
                std::cout << "PVR DXT1 RGB" << std::endl;
                data.channels = 3;
                data.is_compressed_s3tc = true;
                break;
            case 11:
                std::cout << "PVR DXT5 RGBA" << std::endl;
                data.channels = 4;
                data.is_compressed_s3tc = true;
                break;
            case 22:
                std::cout << "PVR ETC2 RGB" << std::endl;
                data.channels = 3;
                data.is_compressed_etc = true;
                break;
            case 23:
                std::cout << "PVR ETC2 RGBA" << std::endl;
                data.channels = 4;
                data.is_compressed_etc = true;
                break;
            default:
                LUCARIA_RUNTIME_ERROR("Invalid S3TC image data")
                break;
            }
            const std::size_t _offset = 52 + *(_data32 + 12);
            const glm::uint8* _data_ptr = _content.data() + _offset;
            const std::size_t _data_size = _content.size() - _offset;
            data.pixels = std::vector<glm::uint8>(_data_ptr, _data_ptr + _data_size);
            data.height = *(_data32 + 6);
            data.width = *(_data32 + 7);

            // ETC2 compression
        } else if (*_data32 == 0x58544BAB) {
            switch (*(_data32 + 7)) {
            case 0x9274:
                std::cout << "KTX ETC2 RGB" << std::endl;
                data.channels = 3;
                data.is_compressed_etc = true;
                break;
            case 0x9278:
                std::cout << "KTX ETC2 RGBA" << std::endl;
                data.channels = 4;
                data.is_compressed_etc = true;
                break;
            default:
                LUCARIA_RUNTIME_ERROR("Invalid ETC2 image data")
                break;
            }
            const std::size_t _offset = sizeof(uint32_t) * 17 + *(_data32 + 15);
            const glm::uint8* _data_ptr = _content.data() + _offset;
            const std::size_t _data_size = _content.size() - _offset;
            data.pixels = std::vector<glm::uint8>(_data_ptr, _data_ptr + _data_size);
            data.width = *(_data32 + 9);
            data.height = *(_data32 + 10);

            // binary raw format
        } else {
            _detail::bytes_stream _stream(data_bytes);
#if LUCARIA_JSON
            cereal::JSONInputArchive _archive(_stream);
#else
            cereal::PortableBinaryInputArchive _archive(_stream);
#endif
            _archive(data);
        }
    }

}

const std::filesystem::path& _resolve_image_path(
    const std::filesystem::path& data_path,
    const std::optional<std::filesystem::path>& etc2_path,
    const std::optional<std::filesystem::path>& s3tc_path)
{
    if (_is_etc2_supported && etc2_path.has_value()) {
        return etc2_path.value();
    } else if (_is_s3tc_supported && s3tc_path.has_value()) {
        return s3tc_path.value();
    } else {
        return data_path;
    }
}

std::vector<std::filesystem::path> _resolve_image_paths(
    const std::array<std::filesystem::path, 6>& data_paths,
    const std::optional<std::array<std::filesystem::path, 6>>& etc2_paths,
    const std::optional<std::array<std::filesystem::path, 6>>& s3tc_paths)
{
    if (_is_etc2_supported && etc2_paths.has_value()) {
        return std::vector<std::filesystem::path>(etc2_paths.value().begin(), etc2_paths.value().end());
    } else if (_is_s3tc_supported && s3tc_paths.has_value()) {
        return std::vector<std::filesystem::path>(s3tc_paths.value().begin(), s3tc_paths.value().end());
    } else {
        return std::vector<std::filesystem::path>(data_paths.begin(), data_paths.end());
    }
}
image::image(const std::vector<char>& data_bytes)
{
    load_data_from_bytes(data, data_bytes);
}

image::image(
    const std::filesystem::path& data_path,
    const std::optional<std::filesystem::path>& etc2_path,
    const std::optional<std::filesystem::path>& s3tc_path)
{
    const std::filesystem::path& _image_path = _resolve_image_path(data_path, etc2_path, s3tc_path);
    _load_bytes(_image_path, [this](const std::vector<char>& _data_bytes) {
        load_data_from_bytes(data, _data_bytes);
    });
}

fetched<image> fetch_image(
    const std::filesystem::path& data_path,
    const std::optional<std::filesystem::path>& etc2_path,
    const std::optional<std::filesystem::path>& s3tc_path)
{
    const std::filesystem::path& _image_path = _resolve_image_path(data_path, etc2_path, s3tc_path);
    std::shared_ptr<std::promise<image>> _promise = std::make_shared<std::promise<image>>();
    _fetch_bytes(_image_path, [_promise](const std::vector<char>& _data_bytes) {
        image _image(_data_bytes);
        _promise->set_value(std::move(_image)); }, true);

    return fetched<image>(_promise->get_future());
}

}