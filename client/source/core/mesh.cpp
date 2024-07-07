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
    _skinned_positions = std::move(other._skinned_positions);
    _positions = std::move(other._positions);
    _bones = std::move(other._bones);
    _weights = std::move(other._weights);
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

mesh_ref::mesh_ref(const mesh_data& data, const bool keep_animation_data)
{
    detail::validate_mesh(data);
    _count = data.count;
    _array_id = detail::create_vertex_array();
    _elements_id = detail::create_elements_buffer(data.indices);
    if (!data.positions.empty()) {
        _attribute_ids[mesh_attribute::position] = detail::create_attribute_buffer(data.positions);
        if (keep_animation_data) {
#if LUCARIA_DEBUG
            if (data.positions.empty()) {
                std::cout << "Requested to keep animation data but positions attribute is empty." << std::endl;
                std::terminate();
            }
            if (data.bones.empty()) {
                std::cout << "Requested to keep animation data but bones attribute is empty." << std::endl;
                std::terminate();
            }
            if (data.weights.empty()) {
                std::cout << "Requested to keep animation data but weights attribute is empty." << std::endl;
                std::terminate();
            }
#endif
            _skinned_positions.resize(_count);
            _positions = data.positions;
            std::cout << "positions size = " << _positions.size() << std::endl;
            _bones = data.bones;
            _weights = data.weights;
        }
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

const std::vector<glm::vec3>& mesh_ref::get_positions() const
{
    return _positions;
}

const std::vector<glm::uvec4>& mesh_ref::get_bones() const
{
    return _bones;
}

const std::vector<glm::vec4>& mesh_ref::get_weights() const
{
    return _weights;
}

void mesh_ref::update_skinned_positions(const std::function<void(std::vector<glm::vec3>&)>& callback)
{
    callback(_skinned_positions);
    glm::uint _attribute_id = _attribute_ids[mesh_attribute::position];
    glm::float32* _attribute_ptr = reinterpret_cast<glm::float32*>(const_cast<glm::vec3*>(_skinned_positions.data()));
    glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::float32) * _count, _attribute_ptr, GL_STATIC_DRAW);
}

mesh_ref load_mesh(const std::filesystem::path& file, const bool keep_positions)
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
    return mesh_ref(_data, keep_positions);
}

std::future<mesh_ref> fetch_mesh(const std::filesystem::path& file, const bool keep_positions)
{
    std::promise<mesh_ref>& _promise = detail::promises[file.string()];
    fetch_file(file.string(), [&_promise, file, keep_positions](std::istringstream& stream) {
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
        _promise.set_value(std::move(mesh_ref(_data, keep_positions)));
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