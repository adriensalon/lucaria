#include <fstream>
#include <iostream>
#include <vector>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <GLES3/gl3.h>

#include <core/cubemap.hpp>
#include <core/texture.hpp>
#include <core/fetch.hpp>

namespace detail {

template <typename T>
bool all_equal(T first)
{
    return true;
}

template <typename T, typename... Args>
bool all_equal(T first, Args... args)
{
    return ((first == args) && ...);
}

static std::unordered_map<std::size_t, std::pair<std::vector<std::pair<cubemap_side, texture_data>>, std::promise<std::shared_ptr<cubemap_ref>>>> promises;

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
    const texture_data& _positive_x = data[static_cast<glm::uint>(cubemap_side::positive_x)];
    const texture_data& _positive_y = data[static_cast<glm::uint>(cubemap_side::positive_y)];
    const texture_data& _positive_z = data[static_cast<glm::uint>(cubemap_side::positive_z)];
    const texture_data& _negative_x = data[static_cast<glm::uint>(cubemap_side::negative_x)];
    const texture_data& _negative_y = data[static_cast<glm::uint>(cubemap_side::negative_y)];
    const texture_data& _negative_z = data[static_cast<glm::uint>(cubemap_side::negative_z)];
    glGenTextures(1, &_cubemap_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _cubemap_id);
    GLenum _format;
    if (detail::all_equal(_positive_x.channels, _positive_y.channels, _positive_z.channels, _negative_x.channels, _negative_y.channels, _negative_z.channels, 3)) {
        _format = GL_RGB;
    } else if (detail::all_equal(_positive_x.channels, _positive_y.channels, _positive_z.channels, _negative_x.channels, _negative_y.channels, _negative_z.channels, 4)) {
        _format = GL_RGBA;
    } 
#if LUCARIA_DEBUG
    else {
        std::cout << "Invalid channels across cubemap textures or channels != 3 or channels != 4" << std::endl;
        std::terminate();
    }
#endif
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const GLubyte* __positive_x_ptr = const_cast<const GLubyte*>(_positive_x.pixels.data());
    const GLubyte* _plus_y_ptr = const_cast<const GLubyte*>(_positive_y.pixels.data());
    const GLubyte* _plus_z_ptr = const_cast<const GLubyte*>(_positive_z.pixels.data());
    const GLubyte* _minus_x_ptr = const_cast<const GLubyte*>(_negative_x.pixels.data());
    const GLubyte* _minus_y_ptr = const_cast<const GLubyte*>(_negative_y.pixels.data());
    const GLubyte* _minus_z_ptr = const_cast<const GLubyte*>(_negative_z.pixels.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, _format, _positive_x.width, _positive_x.height, 0, _format, GL_UNSIGNED_BYTE, __positive_x_ptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, _format, _positive_y.width, _positive_y.height, 0, _format, GL_UNSIGNED_BYTE, _plus_y_ptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, _format, _positive_z.width, _positive_z.height, 0, _format, GL_UNSIGNED_BYTE, _plus_z_ptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, _format, _negative_x.width, _negative_x.height, 0, _format, GL_UNSIGNED_BYTE, _minus_x_ptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, _format, _negative_y.width, _negative_y.height, 0, _format, GL_UNSIGNED_BYTE, _minus_y_ptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, _format, _negative_z.width, _negative_z.height, 0, _format, GL_UNSIGNED_BYTE, _minus_z_ptr);
}

glm::uint cubemap_ref::get_id() const
{
    return _cubemap_id;
}

std::shared_future<std::shared_ptr<cubemap_ref>> fetch_cubemap(const std::array<std::filesystem::path, 6>& texture_paths)
{
    const std::vector<std::filesystem::path> _paths(texture_paths.begin(), texture_paths.end());
    const std::size_t _hash = compute_hash_files(_paths);
    std::pair<std::vector<std::pair<cubemap_side, texture_data>>, std::promise<std::shared_ptr<cubemap_ref>>>& _promise_pair = detail::promises[_hash];
    fetch_files(_paths, [&_promise_pair](const std::size_t _side_index, const std::size_t, std::istringstream& stream) {
        _promise_pair.first.emplace_back(static_cast<cubemap_side>(_side_index), std::move(load_texture_data(stream)));
        if (_promise_pair.first.size() == 6) {
            std::array<texture_data, 6> _textures;
            for (glm::uint _index = 0; _index < 6; ++_index) {
                const std::pair<const cubemap_side, texture_data>& _pair = _promise_pair.first[_index];
                _textures[static_cast<glm::uint>(_pair.first)] = std::move(_pair.second);
            }
            _promise_pair.second.set_value(std::move(std::make_shared<cubemap_ref>(_textures)));
        }
    });
    return _promise_pair.second.get_future();
}