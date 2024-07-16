#include <fstream>
#include <iostream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <GLES3/gl3.h>

#include <core/mesh.hpp>
#include <glue/fetch.hpp>

namespace detail {

void validate_mesh(const mesh_data& data)
{
}

glm::uint create_vertex_array()
{
    glm::uint _array_id;
    glGenVertexArrays(1, &_array_id);
    glBindVertexArray(_array_id);
    return _array_id;
}

glm::uint create_attribute_buffer(const std::vector<glm::vec2>& attribute)
{
    glm::uint _attribute_id;
    glm::float32* _attribute_ptr = reinterpret_cast<glm::float32*>(const_cast<glm::vec2*>(attribute.data()));
    glGenBuffers(1, &_attribute_id);
    glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::float32) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
#if LUCARIA_DEBUG
    std::cout << "Created VEC2 ARRAY_BUFFER buffer of size " << attribute.size()
              << " with id " << _attribute_id << std::endl;
#endif
    return _attribute_id;
}

glm::uint create_empty_vec3_attribute_buffer(const glm::uint count)
{
    glm::uint _attribute_id;
    glGenBuffers(1, &_attribute_id);
    glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::float32) * count, nullptr, GL_STATIC_DRAW);
#if LUCARIA_DEBUG
    std::cout << "Created EMPTY VEC3 ARRAY_BUFFER buffer of size " << count
              << " with id " << _attribute_id << std::endl;
#endif
    return _attribute_id;
}

glm::uint create_attribute_buffer(const std::vector<glm::vec3>& attribute)
{
    glm::uint _attribute_id;
    glm::float32* _attribute_ptr = reinterpret_cast<glm::float32*>(const_cast<glm::vec3*>(attribute.data()));
    glGenBuffers(1, &_attribute_id);
    glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::float32) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
#if LUCARIA_DEBUG
    std::cout << "Created VEC3 ARRAY_BUFFER buffer of size " << attribute.size()
              << " with id " << _attribute_id << std::endl;
#endif
    return _attribute_id;
}

glm::uint create_attribute_buffer(const std::vector<glm::vec4>& attribute)
{
    glm::uint _attribute_id;
    glm::float32* _attribute_ptr = reinterpret_cast<glm::float32*>(const_cast<glm::vec4*>(attribute.data()));
    glGenBuffers(1, &_attribute_id);
    glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::float32) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
#if LUCARIA_DEBUG
    std::cout << "Created VEC4 ARRAY_BUFFER buffer of size " << attribute.size()
              << " with id " << _attribute_id << std::endl;
#endif
    return _attribute_id;
}

glm::uint create_elements_buffer(const std::vector<glm::uvec3>& indices)
{
    glm::uint _elements_id;
    glm::uint* _indices_ptr = reinterpret_cast<glm::uint*>(const_cast<glm::uvec3*>(indices.data()));
    glGenBuffers(1, &_elements_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * indices.size() * sizeof(glm::uint), _indices_ptr, GL_STATIC_DRAW);
    return _elements_id;
}

static std::unordered_map<std::string, std::promise<std::shared_ptr<mesh_ref>>> promises;

}

mesh_ref::mesh_ref(mesh_ref&& other)
{
    *this = std::move(other);
}

mesh_ref& mesh_ref::operator=(mesh_ref&& other)
{
    _count = other._count;
    _array_id = other._array_id;
    _elements_id = other._elements_id;
    _attribute_ids = std::move(other._attribute_ids);
    _is_instanced = true;
    other._is_instanced = false;
    return *this;
}

mesh_ref::~mesh_ref()
{
    if (_is_instanced) {
        glDeleteBuffers(1, &_array_id);
        glDeleteBuffers(1, &_elements_id);
        for (const std::pair<const mesh_attribute, glm::uint>& _pair : _attribute_ids) {
            glDeleteBuffers(1, &_pair.second);
        }
    }
}

mesh_ref::mesh_ref(const mesh_data& data)
{
    detail::validate_mesh(data);
    _count = data.count;
    _array_id = detail::create_vertex_array();
    _elements_id = detail::create_elements_buffer(data.indices);
    if (!data.positions.empty()) {
        _attribute_ids[mesh_attribute::position] = detail::create_attribute_buffer(data.positions);
    } else {
        _attribute_ids[mesh_attribute::position] = detail::create_empty_vec3_attribute_buffer(data.count);
    }
    if (!data.colors.empty()) {
        _attribute_ids[mesh_attribute::color] = detail::create_attribute_buffer(data.colors);
    }
    if (!data.normals.empty()) {
        _attribute_ids[mesh_attribute::normal] = detail::create_attribute_buffer(data.normals);
    }
    if (!data.tangents.empty()) {
        _attribute_ids[mesh_attribute::tangent] = detail::create_attribute_buffer(data.tangents);
    }
    if (!data.bitangents.empty()) {
        _attribute_ids[mesh_attribute::bitangent] = detail::create_attribute_buffer(data.bitangents);
    }
    if (!data.texcoords.empty()) {
        _attribute_ids[mesh_attribute::texcoord] = detail::create_attribute_buffer(data.texcoords);
    }
    _is_instanced = true;
}

void mesh_ref::update_positions(const std::vector<glm::vec3>& new_positions)
{
    glm::uint _attribute_id = _attribute_ids.at(mesh_attribute::position);
    glm::float32* _attribute_ptr = reinterpret_cast<glm::float32*>(const_cast<glm::vec3*>(new_positions.data()));
    glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::float32) * _count, _attribute_ptr, GL_STATIC_DRAW);
}

std::unordered_map<mesh_attribute, glm::uint> mesh_ref::get_buffer_ids() const
{
    return _attribute_ids;
}

glm::uint mesh_ref::get_array_id() const
{
    return _array_id;
}

glm::uint mesh_ref::get_count() const
{
    return _count;
}

mesh_data load_mesh_data(std::istringstream& mesh_stream)
{
    mesh_data _data;
    {
#if LUCARIA_JSON
        cereal::JSONInputArchive _archive(mesh_stream);
#else
        cereal::PortableBinaryInputArchive _archive(mesh_stream);
#endif
        _archive(_data);
    }
    return _data;
}

std::shared_future<std::shared_ptr<mesh_ref>> fetch_mesh(const std::filesystem::path& mesh_path)
{
    std::promise<std::shared_ptr<mesh_ref>>& _promise = detail::promises[mesh_path.string()];
    fetch_file(mesh_path, [&_promise](std::istringstream& stream) {
        _promise.set_value(std::move(std::make_shared<mesh_ref>(load_mesh_data(stream))));
    });
    return _promise.get_future();
}
