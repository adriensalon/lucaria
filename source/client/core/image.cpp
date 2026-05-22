#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/database.hpp>
#include <lucaria/core/image.hpp>
#include <lucaria/core/stream.hpp>
#include <lucaria/core/window.hpp>

namespace lucaria {

const std::filesystem::path& _resolve_image_path(const std::filesystem::path& data_path, const std::optional<std::filesystem::path>& etc2_path, const std::optional<std::filesystem::path>& s3tc_path);
std::vector<std::filesystem::path> _resolve_image_paths(const std::array<std::filesystem::path, 6>& data_paths, const std::optional<std::array<std::filesystem::path, 6>>& etc2_paths, const std::optional<std::array<std::filesystem::path, 6>>& s3tc_paths);

const std::filesystem::path& _resolve_image_path(
    const std::filesystem::path& data_path,
    const std::optional<std::filesystem::path>& etc2_path,
    const std::optional<std::filesystem::path>& s3tc_path)
{
    if (detail::engine_window().is_etc2_supported && etc2_path.has_value()) {
        return etc2_path.value();
    } else if (detail::engine_window().is_s3tc_supported && s3tc_path.has_value()) {
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
    if (detail::engine_window().is_etc2_supported && etc2_paths.has_value()) {
        return std::vector<std::filesystem::path>(etc2_paths.value().begin(), etc2_paths.value().end());
    } else if (detail::engine_window().is_s3tc_supported && s3tc_paths.has_value()) {
        return std::vector<std::filesystem::path>(s3tc_paths.value().begin(), s3tc_paths.value().end());
    } else {
        return std::vector<std::filesystem::path>(data_paths.begin(), data_paths.end());
    }
}

namespace detail {

    namespace {

        static void _load_image_bytes(image_data& data, const std::vector<char>& bytes)
        {
            const std::vector<uint8_t>& _content = *(reinterpret_cast<const std::vector<uint8_t>*>(&bytes));
            const uint32_t* _data32 = (uint32_t*)_content.data();
            data.is_compressed_etc = false;
            data.is_compressed_s3tc = false;

            // S3TC compression
            if (*_data32 == 0x03525650) {
                switch (*(_data32 + 2)) {
                case 7:
                    // std::cout << "PVR DXT1 RGB" << std::endl;
                    data.channels = 3;
                    data.is_compressed_s3tc = true;
                    break;
                case 11:
                    // std::cout << "PVR DXT5 RGBA" << std::endl;
                    data.channels = 4;
                    data.is_compressed_s3tc = true;
                    break;
                case 22:
                    // std::cout << "PVR ETC2 RGB" << std::endl;
                    data.channels = 3;
                    data.is_compressed_etc = true;
                    break;
                case 23:
                    // std::cout << "PVR ETC2 RGBA" << std::endl;
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
                    // std::cout << "KTX ETC2 RGB" << std::endl;
                    data.channels = 3;
                    data.is_compressed_etc = true;
                    break;
                case 0x9278:
                    // std::cout << "KTX ETC2 RGBA" << std::endl;
                    data.channels = 4;
                    data.is_compressed_etc = true;
                    break;
                default:
                    LUCARIA_RUNTIME_ERROR("Invalid ETC2 image data")
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
#if defined(LUCARIA_JSON_ASSETS)
                cereal::JSONInputArchive _archive(_stream);
#else
                cereal::PortableBinaryInputArchive _archive(_stream);
#endif
                _archive(data);
            }
        }

        static async_container<image_implementation> _fetch_image_async(
            const std::filesystem::path& data_path,
            const std::optional<std::filesystem::path>& etc2_path,
            const std::optional<std::filesystem::path>& s3tc_path)
        {
            const std::filesystem::path& _image_path = _resolve_image_path(data_path, etc2_path, s3tc_path);
            std::shared_ptr<std::promise<image_implementation>> _promise = std::make_shared<std::promise<image_implementation>>();
            fetch_bytes(_image_path, [_promise](const std::vector<char>& _bytes) {
        image_implementation _image(_bytes);
        _promise->set_value(std::move(_image)); }, true);

            return async_container<image_implementation>(_promise->get_future());
        }

    }

    image_implementation::image_implementation(const std::vector<char>& bytes)
        : origin(image_origin::path)
    {
        _load_image_bytes(data, bytes);
    }

    image_implementation::image_implementation(image_data&& data)
        : origin(image_origin::data)
        , data(std::move(data))
    {
    }

	image_recipe make_recipe(const implementation_container<image_implementation>& container)
    {
        const image_implementation& _image = container.fetched.value();

        if (_image.origin == image_origin::path) {
            return image_path_recipe { container.origin_path.value(), std::nullopt, std::nullopt };
        }

        else if (_image.origin == image_origin::data) {
            return image_data_recipe { _image.data };
        }

        else {
            LUCARIA_RUNTIME_ERROR("Implementation error");
            return {};
        }
    }

}

image_object image_object::fetch(
    const std::filesystem::path& path,
    const std::optional<std::filesystem::path>& etc2_path,
    const std::optional<std::filesystem::path>& s3tc_path)
{
	image_object _image = {};
    _image._resource = detail::engine_resources().images.get_or_create_by_path(path, [&] {
        return detail::_fetch_image_async(path, etc2_path, s3tc_path);
    });
    _image._manager = &detail::engine_resources().images;
    _image._refcount.emplace();
    return _image;
}

bool image_object::has_value() const
{
    return _resource && _resource->fetched.has_value();
}

image_object::operator bool() const
{
    return has_value();
}

}