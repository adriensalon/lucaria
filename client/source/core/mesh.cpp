#include <fstream>
#include <iostream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>

#include <core/mesh.hpp>
#include <glue/fetch.hpp>

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

GLuint create_elements_buffer(const std::vector<GLuint>& indices)
{
    GLuint _elements_id;
    glGenBuffers(1, &_elements_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &(indices[0]), GL_STATIC_DRAW);
    return _elements_id;
}

static std::unordered_map<std::string, std::promise<mesh_ref>> promises;
static std::unordered_map<std::size_t, std::pair<std::vector<mesh_ref>, std::promise<std::vector<mesh_data>>>> vector_promises;

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
    _must_destroy = true;
    other._must_destroy = false;
    return *this;
}

mesh_ref::~mesh_ref()
{
    if (_must_destroy) {
        // TODO
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
    _must_destroy = true;
}

std::unordered_map<mesh_attribute, GLuint> mesh_ref::get_buffer_ids() const
{
    return _attribute_ids;
}

void mesh_ref::update_buffer(const mesh_attribute attribute, const std::vector<float>& data)
{
    if (_attribute_ids.find(attribute) != _attribute_ids.end()) {
        glDeleteBuffers(1, &_attribute_ids[attribute]);
    }
    _attribute_ids[attribute] = detail::create_attribute_buffer(data);
}

GLuint mesh_ref::get_array_id() const
{
    return _array_id;
}

GLuint mesh_ref::get_count() const
{
    return _count;
}

mesh_ref load_mesh(const std::filesystem::path& file)
{
#if LUCARIA_DEBUG
    if (!std::filesystem::is_regular_file(file)) {
        std::cout << "Invalid mesh path " << file << std::endl;
        std::terminate();
    }
#endif
    mesh_data _data;
    {
#if LUCARIA_JSON
        std::ifstream _fstream(file);
            
        cereal::JSONInputArchive _archive(_fstream);
#else
        std::ifstream _fstream(file, std::ios::binary);
        cereal::PortableBinaryInputArchive _archive(_fstream);
#endif
        _archive(_data);
    }
#if LUCARIA_DEBUG
    std::cout << "Loaded mesh data from " << file << " ("
              << _data.count << " vertices)" << std::endl;
#endif
    return mesh_ref(_data);
}

std::future<mesh_ref> fetch_mesh(const std::filesystem::path& file)
{
    std::promise<mesh_ref>& _promise = detail::promises[file.generic_string()];
    fetch_file(file.string(), [&_promise, file](std::istringstream& stream) {
        mesh_data _data;
        {
#if LUCARIA_JSON
            cereal::JSONInputArchive _archive(stream);
#else
            cereal::PortableBinaryInputArchive _archive(stream);
#endif
            _archive(_data);
        }
        
#if LUCARIA_DEBUG
        std::cout << "Loaded mesh data from " << file << " ("
                  << _data.count << " vertices)" << std::endl;
#endif
        _promise.set_value(std::move(mesh_ref(_data)));
    });
    return _promise.get_future();
}

// std::future<std::vector<mesh_data>> fetch_meshes(const std::vector<std::filesystem::path>& files)
// {
//     const std::size_t _hash = compute_hash_files(files);
//     std::promise<std::vector<mesh_data>>& _promise = detail::vector_promises[_hash].second;
//     std::vector<mesh_data>& _data = detail::vector_promises[_hash].first;
//     fetch_files(files, [&_data, &_promise, files, _hash](const std::size_t index, const std::size_t size, std::istringstream& stream) {
//         {
// #if LUCARIA_JSON
//             cereal::JSONInputArchive _archive(stream);
// #else
//             cereal::PortableBinaryInputArchive _archive(stream);
// #endif
//             _archive(_data.emplace_back());
            
//         }
// #if LUCARIA_DEBUG
//         std::cout << "Loaded mesh data from " << files[index].generic_string() << " ("
//                   << _data[index].count << " vertices)" << std::endl;
// #endif
//         if (_data.size() == size) {
//             _promise.set_value(std::move(_data));
//         }
//     });
//     return _promise.get_future();
// }