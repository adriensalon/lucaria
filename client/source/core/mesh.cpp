#include <fstream>
#include <iostream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

#include <core/mesh.hpp>

namespace detail {

void validate_mesh(const mesh_data& data)
{
}

GLuint create_vertex_array()
{
    GLuint _array_id;
    glGenVertexArrays(1, &_array_id);
    glBindVertexArray(_array_id);
    return _array_id;
}

GLuint create_attribute_buffer(const std::vector<GLfloat>& attribute)
{
    GLuint _attribute_id;
    GLfloat* _attribute_ptr = const_cast<GLfloat*>(attribute.data());
    glGenBuffers(1, &_attribute_id);
    glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
#if LUCARIA_DEBUG
    std::cout << "Created ARRAY_BUFFER buffer of size " << attribute.size()
              << " with id " << _attribute_id << std::endl;
#endif
    return _attribute_id;
}

void emplace_attribute_buffer(GLuint& attribute, const std::vector<GLfloat>& data)
{
    if (!data.empty()) {
        attribute = create_attribute_buffer(data);
    }
}

GLuint create_elements_buffer(const std::vector<GLuint>& indices)
{
    GLuint _elements_id;
    glGenBuffers(1, &_elements_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &(indices[0]), GL_STATIC_DRAW);
    return _elements_id;
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
}

mesh_ref::~mesh_ref()
{
}

std::unordered_map<mesh_attribute, GLuint> mesh_ref::get_buffer_ids() const
{
    return _attribute_ids;
}

GLuint mesh_ref::get_array_id() const
{
    return _array_id;
}

GLuint mesh_ref::get_count() const
{
    return _count;
}

mesh_data load_mesh(const std::filesystem::path& file)
{
#if LUCARIA_DEBUG
    if (!std::filesystem::is_regular_file(file)) {
        std::cout << "Invalid mesh path " << file << std::endl;
        std::terminate();
    }
#endif
    mesh_data _data;
    std::ifstream _fstream(file, std::ios::binary);
    cereal::PortableBinaryInputArchive _archive(_fstream);
    _archive(_data);
#if LUCARIA_DEBUG
    std::cout << "Loaded mesh data from " << file << " ("
              << _data.count << " vertices)" << std::endl;
#endif
    return _data;
}