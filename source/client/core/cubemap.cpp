#include <fstream>
#include <iostream>
#include <vector>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>

#include <lucaria/core/cubemap.hpp>
#include <lucaria/core/texture.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/graphics.hpp>
#include <lucaria/core/hash.hpp>
#include <lucaria/core/window.hpp>

// constexpr static GLenum COMPRESSED_R11_EAC = 0x9270;
// constexpr static GLenum COMPRESSED_SIGNED_R11_EAC = 0x9271;
// constexpr static GLenum COMPRESSED_RG11_EAC = 0x9272;
// constexpr static GLenum COMPRESSED_SIGNED_RG11_EAC = 0x9273;
constexpr static GLenum COMPRESSED_RGB8_ETC2 = 0x9274;
// constexpr static GLenum COMPRESSED_SRGB8_ETC2 = 0x9275;
// constexpr static GLenum COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9276;
// constexpr static GLenum COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277;
constexpr static GLenum COMPRESSED_RGBA8_ETC2_EAC = 0x9278;
// constexpr static GLenum COMPRESSED_SRGB8_ALPHA8_ETC2_EAC = 0x9279;

constexpr static GLenum COMPRESSED_RGB_S3TC_DXT1_EXT = 0x83F0;
// constexpr static GLenum COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1;
// constexpr static GLenum COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2;
constexpr static GLenum COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;

namespace detail {

const std::array<GLenum, 6> cubemap_enums = {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

static std::unordered_map<std::size_t, std::pair<std::vector<std::pair<cubemap_side, image_data>>, std::promise<std::shared_ptr<cubemap_ref>>>> promises;

}

cubemap_ref::cubemap_ref(cubemap_ref&& other)
{
    *this = std::move(other);
}

cubemap_ref& cubemap_ref::operator=(cubemap_ref&& other)
{
    _cubemap_id = other._cubemap_id;
    _is_instanced = true;
    other._is_instanced = false;
    return *this;
}

cubemap_ref::~cubemap_ref()
{
    if (_is_instanced) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glDeleteTextures(1, &_cubemap_id);
    }
}

cubemap_ref::cubemap_ref(const cubemap_data& data)
{
    glGenTextures(1, &_cubemap_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _cubemap_id);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    for (glm::uint _index = 0; _index < 6; ++_index) {
        const image_data& _data = data[_index];
        const GLubyte* _pixels_ptr = _data.pixels.data();
        const GLenum _side_enum = detail::cubemap_enums[_index];
        switch (_data.channels) {
        case 3:
            if (_data.is_compressed_etc && get_is_etc_supported()) {
                glCompressedTexImage2D(_side_enum, 0, COMPRESSED_RGB8_ETC2, _data.width, _data.height, 0, _data.pixels.size(), _pixels_ptr);
            } else if (_data.is_compressed_s3tc && get_is_s3tc_supported()) {
                glCompressedTexImage2D(_side_enum, 0, COMPRESSED_RGB_S3TC_DXT1_EXT, _data.width, _data.height, 0, _data.pixels.size(), _pixels_ptr);
            } else {
                glTexImage2D(_side_enum, 0, GL_RGB, _data.width, _data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
            }
            break;
        case 4:
            if (_data.is_compressed_etc && get_is_etc_supported()) {
                glCompressedTexImage2D(_side_enum, 0, COMPRESSED_RGBA8_ETC2_EAC, _data.width, _data.height, 0, _data.pixels.size(), _pixels_ptr);
            } else if (_data.is_compressed_s3tc && get_is_s3tc_supported()) {
                glCompressedTexImage2D(_side_enum, 0, COMPRESSED_RGBA_S3TC_DXT5_EXT, _data.width, _data.height, 0, _data.pixels.size(), _pixels_ptr);
            } else {
                glTexImage2D(_side_enum, 0, GL_RGBA, _data.width, _data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels_ptr);
            }
            break;
        default:
    #if LUCARIA_DEBUG
            std::cout << "Invalid channels count, must be 3 or 4" << std::endl;
            std::terminate();
    #endif
            break;
        }
    }
}

glm::uint cubemap_ref::get_id() const
{
    return _cubemap_id;
}

std::shared_future<std::shared_ptr<cubemap_ref>> fetch_cubemap(const std::array<std::filesystem::path, 6>& image_paths, const std::optional<std::array<std::filesystem::path, 6>>& etc_image_paths, const std::optional<std::array<std::filesystem::path, 6>>& s3tc_image_paths)
{
    bool _is_compressed;
    std::array<std::filesystem::path, 6> _image_paths;
    if (get_is_etc_supported() && etc_image_paths) {
        _is_compressed = true;
        _image_paths = etc_image_paths.value();
    } else if (get_is_s3tc_supported() && s3tc_image_paths) {
        _is_compressed = true;
        _image_paths = s3tc_image_paths.value();
    } else {
        _is_compressed = false;
        _image_paths = image_paths;
    }
    const std::vector<std::filesystem::path> _paths(_image_paths.begin(), _image_paths.end());
    const std::size_t _hash = path_vector_hash()(_paths);
    std::pair<std::vector<std::pair<cubemap_side, image_data>>, std::promise<std::shared_ptr<cubemap_ref>>>& _promise_pair = detail::promises[_hash];
    fetch_files(_paths, [&_promise_pair, _is_compressed](const std::size_t _side_index, const std::size_t, const std::vector<char>& image_bytes) {
        if (_is_compressed) {
            _promise_pair.first.emplace_back(static_cast<cubemap_side>(_side_index), load_compressed_image_data(image_bytes));
        } else {
            _promise_pair.first.emplace_back(static_cast<cubemap_side>(_side_index), load_image_data(image_bytes));
        }        
        if (_promise_pair.first.size() == 6) {
            std::array<image_data, 6> _images;
            for (glm::uint _index = 0; _index < 6; ++_index) {
                const std::pair<const cubemap_side, image_data>& _pair = _promise_pair.first[_index];
                _images[static_cast<glm::uint>(_pair.first)] = std::move(_pair.second);
            }
            _promise_pair.second.set_value(std::make_shared<cubemap_ref>(_images));
        }
    });
    return _promise_pair.second.get_future();
}

void clear_cubemap_fetches()
{
    detail::promises.clear();
}