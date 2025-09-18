#include <fstream>
#include <iostream>
#include <set>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>

#include <lucaria/core/graphics.hpp>
#include <lucaria/core/mesh.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/load.hpp>

namespace detail {

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

// glm::uint create_empty_vec3_attribute_buffer(const glm::uint vertex_count)
// {
//     glm::uint _attribute_id;
//     glGenBuffers(1, &_attribute_id);
//     glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
//     glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::float32) * vertex_count, nullptr, GL_STATIC_DRAW);
// #if LUCARIA_DEBUG
//     std::cout << "Created EMPTY VEC3 ARRAY_BUFFER buffer of size " << vertex_count
//               << " with id " << _attribute_id << std::endl;
// #endif
//     return _attribute_id;
// }

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

glm::uint create_attribute_buffer(const std::vector<glm::uvec4>& attribute)
{
    glm::uint _attribute_id;
    glm::uint* _attribute_ptr = reinterpret_cast<glm::uint*>(const_cast<glm::uvec4*>(attribute.data()));
    glGenBuffers(1, &_attribute_id);
    glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::uint) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
#if LUCARIA_DEBUG
    std::cout << "Created UVEC4 ARRAY_BUFFER buffer of size " << attribute.size()
              << " with id " << _attribute_id << std::endl;
#endif
    return _attribute_id;
}

glm::uint create_attribute_buffer(const std::vector<glm::ivec4>& attribute)
{
    glm::uint _attribute_id;
    glm::int32* _attribute_ptr = reinterpret_cast<glm::int32*>(const_cast<glm::ivec4*>(attribute.data()));
    glGenBuffers(1, &_attribute_id);
    glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::int32) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
#if LUCARIA_DEBUG
    std::cout << "Created IVEC4 ARRAY_BUFFER buffer of size " << attribute.size()
              << " with id " << _attribute_id << std::endl;
#endif
    return _attribute_id;
}

glm::uint create_elements_buffer(const std::vector<glm::uvec2>& indices)
{
    glm::uint _elements_id;
    glm::uint* _indices_ptr = reinterpret_cast<glm::uint*>(const_cast<glm::uvec2*>(indices.data()));
    glGenBuffers(1, &_elements_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * indices.size() * sizeof(glm::uint), _indices_ptr, GL_STATIC_DRAW);
    return _elements_id;
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

std::vector<glm::uvec2> generate_line_indices(const std::vector<glm::uvec3>& triangle_indices) 
{
    std::set<std::pair<glm::uint, glm::uint>> _edges;
    for (const glm::uvec3& _triangle : triangle_indices) {
        _edges.insert(std::minmax(_triangle.x, _triangle.y));
        _edges.insert(std::minmax(_triangle.y, _triangle.z));
        _edges.insert(std::minmax(_triangle.z, _triangle.x));
    }
    std::vector<glm::uvec2> _line_indices;
    for (const auto& _edge : _edges) {
        _line_indices.emplace_back(_edge.first, _edge.second);
    }
    return _line_indices;
}


static std::unordered_map<std::string, std::promise<std::shared_ptr<mesh_ref>>> promises;

}

mesh_ref::mesh_ref(mesh_ref&& other)
{
    *this = std::move(other);
}

mesh_ref& mesh_ref::operator=(mesh_ref&& other)
{
    _indices_count = other._indices_count;
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

mesh_ref::mesh_ref(const geometry_data& data)
{
    _indices_count = 3 * data.indices.size();
    _array_id = detail::create_vertex_array();
    _elements_id = detail::create_elements_buffer(data.indices);
    _invposes = data.invposes;
    if (!data.positions.empty()) {
        _attribute_ids[mesh_attribute::position] = detail::create_attribute_buffer(data.positions);
        std::cout << "Creating positions attribute" << std::endl;
    }
    if (!data.colors.empty()) {
        _attribute_ids[mesh_attribute::color] = detail::create_attribute_buffer(data.colors);
        std::cout << "Creating colors attribute" << std::endl;
    }
    if (!data.normals.empty()) {
        _attribute_ids[mesh_attribute::normal] = detail::create_attribute_buffer(data.normals);
        std::cout << "Creating normals attribute" << std::endl;
    }
    if (!data.tangents.empty()) {
        _attribute_ids[mesh_attribute::tangent] = detail::create_attribute_buffer(data.tangents);
        std::cout << "Creating tangents attribute" << std::endl;
    }
    if (!data.bitangents.empty()) {
        _attribute_ids[mesh_attribute::bitangent] = detail::create_attribute_buffer(data.bitangents);
        std::cout << "Creating bitangents attribute" << std::endl;
    }
    if (!data.texcoords.empty()) {
        _attribute_ids[mesh_attribute::texcoord] = detail::create_attribute_buffer(data.texcoords);
        std::cout << "Creating texcoords attribute" << std::endl;
    }
    if (!data.bones.empty()) {
        _attribute_ids[mesh_attribute::bones] = detail::create_attribute_buffer(data.bones);
        std::cout << "Creating bones attribute" << std::endl;
    }
    if (!data.weights.empty()) {
        _attribute_ids[mesh_attribute::weights] = detail::create_attribute_buffer(data.weights);
        std::cout << "Creating weights attribute" << std::endl;
    }
    _is_instanced = true;
}

std::unordered_map<mesh_attribute, glm::uint> mesh_ref::get_buffer_ids() const
{
    return _attribute_ids;
}

glm::uint mesh_ref::get_array_id() const
{
    return _array_id;
}

glm::uint mesh_ref::get_indices_count() const
{
    return _indices_count;
}

const std::vector<glm::mat4>& mesh_ref::get_invposes() const
{
    return _invposes;
}

geometry_data load_geometry_data(const std::vector<char>& geometry_bytes)
{
    geometry_data _data;
    {
        raw_input_stream _stream(geometry_bytes);
#if LUCARIA_JSON
        cereal::JSONInputArchive _archive(_stream);
#else
        cereal::PortableBinaryInputArchive _archive(_stream);
#endif
        _archive(_data);
    }
    return _data;
}

std::shared_future<std::shared_ptr<mesh_ref>> fetch_mesh(const std::filesystem::path& mesh_path)
{
    std::promise<std::shared_ptr<mesh_ref>>& _promise = detail::promises[mesh_path.string()];
    fetch_file(mesh_path, [&_promise](const std::vector<char>& geometry_bytes) {
        _promise.set_value(std::make_shared<mesh_ref>(load_geometry_data(geometry_bytes)));
    });
    return _promise.get_future();
}

void clear_mesh_fetches()
{
    detail::promises.clear();
}

#if LUCARIA_GUIZMO
    
guizmo_mesh_ref::guizmo_mesh_ref(guizmo_mesh_ref&& other)
{
    *this = std::move(other);
}

guizmo_mesh_ref& guizmo_mesh_ref::operator=(guizmo_mesh_ref&& other)
{
    _indices_count = other._indices_count;
    _array_id = other._array_id;
    _elements_id = other._elements_id;
    _positions_id = other._positions_id;
    _is_instanced = true;
    other._is_instanced = false;
    return *this;
}

guizmo_mesh_ref::~guizmo_mesh_ref()
{
    if (_is_instanced) {
        glDeleteBuffers(1, &_array_id);
        glDeleteBuffers(1, &_elements_id);
        glDeleteBuffers(1, &_positions_id);
    }
}

guizmo_mesh_ref::guizmo_mesh_ref(const geometry_data& data)
{
    _array_id = detail::create_vertex_array();
    std::vector<glm::uvec2> _line_indices = detail::generate_line_indices(data.indices);
    _indices_count = 2 * _line_indices.size();
    _elements_id = detail::create_elements_buffer(_line_indices);
    _positions_id = detail::create_attribute_buffer(data.positions);
    _is_instanced = true;
}

guizmo_mesh_ref::guizmo_mesh_ref(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices)
{
    _array_id = detail::create_vertex_array();
    _indices_count = indices.size() * 2;
    _elements_id = detail::create_elements_buffer(indices);
    _positions_id = detail::create_attribute_buffer(positions);
    _is_instanced = true;
}

void guizmo_mesh_ref::update(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices)
{
    glBindVertexArray(_array_id);
    glBindBuffer(GL_ARRAY_BUFFER, _positions_id);
    glBufferData(GL_ARRAY_BUFFER, 3 * positions.size() * sizeof(glm::float32), positions.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * indices.size() * sizeof(glm::uint), indices.data(), GL_STATIC_DRAW);
    _indices_count = indices.size() * 2;
}

glm::uint guizmo_mesh_ref::get_positions_id() const
{
    return _positions_id;
}

glm::uint guizmo_mesh_ref::get_array_id() const
{
    return _array_id;
}

glm::uint guizmo_mesh_ref::get_indices_count() const
{
    return _indices_count;
}


#endif