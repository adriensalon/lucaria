#include <glm/gtc/matrix_inverse.hpp>

#include <lucaria/core/opengl.hpp>
#include <lucaria/core/viewport.hpp>

namespace lucaria {
namespace {

    [[nodiscard]] static std::vector<glm::vec2> invert_texcoords(const std::vector<glm::vec2>& texcoords)
    {
        std::vector<glm::vec2> _inverted;
        _inverted.reserve(texcoords.size());
        for (const glm::vec2& _texcoord : texcoords) {
            _inverted.push_back({ _texcoord.x, 1.0f - _texcoord.y });
        }
        return _inverted;
    }

    static void make_onb_frisvad_branchless_pole_y(const glm::vec3& n, glm::vec3& u, glm::vec3& v)
    {
        const glm::vec3 _np(n.x, n.z, n.y);
        const glm::float32 _sign = std::copysign(1.0f, _np.z);
        glm::float32 _denominator = _sign + _np.z;
        _denominator = (std::fabs(_denominator) < 1e-12f) ? ((_denominator < 0.0f) ? -1e-12f : 1e-12f) : _denominator;

        const glm::float32 _a = -1.0f / _denominator;
        const glm::float32 _b = _np.x * _np.y * _a;
        const glm::vec3 _up(1.0f + _sign * _np.x * _np.x * _a, _sign * _b, -_sign * _np.x);
        const glm::vec3 _vp(_b, _sign + _np.y * _np.y * _a, -_np.y);

        u = glm::normalize(glm::vec3(_up.x, _up.z, _up.y));
        v = glm::normalize(glm::vec3(_vp.x, _vp.z, _vp.y));
        if (glm::dot(glm::cross(u, v), n) < 0.0f) {
            v = -v;
        }

        if (u.x < 0) {
            u = -u;
            v = -v;
        }
    }

    [[nodiscard]] static glm::uvec2 compute_size_planar_fast(
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::uvec3>& indices,
        const glm::float32 pixels_per_meter,
        const glm::uint min_side = 16,
        const glm::uint max_side = 4096)
    {
        glm::vec3 _face_normal(0, 0, 1);
        for (const glm::uvec3& _triangle : indices) {
            const glm::vec3 a = positions[_triangle.x];
            const glm::vec3 b = positions[_triangle.y];
            const glm::vec3 c = positions[_triangle.z];
            const glm::vec3 n = glm::cross(b - a, c - a);
            if (glm::length(n) > 1e-10f) { // threshold must be sqrt of before
                _face_normal = glm::normalize(n);
                break;
            }
        }

        glm::vec3 _u, _v;
        make_onb_frisvad_branchless_pole_y(_face_normal, _u, _v);

        glm::float32 _min_u = FLT_MAX;
        glm::float32 _max_u = -FLT_MAX;
        glm::float32 _min_v = FLT_MAX;
        glm::float32 _max_v = -FLT_MAX;
        for (const glm::vec3& _position : positions) {
            const glm::float32 u = glm::dot(_position, _u);
            const glm::float32 v = glm::dot(_position, _v);
            _min_u = std::min(_min_u, u);
            _max_u = std::max(_max_u, u);
            _min_v = std::min(_min_v, v);
            _max_v = std::max(_max_v, v);
        }

        const glm::float32 _width_meters = std::max(0.0f, _max_u - _min_u);
        const glm::float32 _height_meters = std::max(0.0f, _max_v - _min_v);        
        glm::uint _width_pixels = static_cast<glm::uint>(std::lround(pixels_per_meter * _width_meters));
        glm::uint _height_pixels = static_cast<glm::uint>(std::lround(pixels_per_meter * _height_meters));
        _width_pixels = glm::clamp(_width_pixels, min_side, max_side);
        _height_pixels = glm::clamp(_height_pixels, min_side, max_side);
        return { _width_pixels, _height_pixels };
    }

    [[nodiscard]] static glm::uint create_vertex_array()
    {
        glm::uint _array_handle;
        glGenVertexArrays(1, &_array_handle);
        glBindVertexArray(_array_handle);
        return _array_handle;
    }

    [[nodiscard]] static glm::uint create_elements_buffer(const std::vector<glm::uvec3>& indices)
    {
        glm::uint _elements_handle;
        glm::uint* _indices_ptr = reinterpret_cast<glm::uint*>(const_cast<glm::uvec3*>(indices.data()));
        glGenBuffers(1, &_elements_handle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements_handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * indices.size() * sizeof(glm::uint), _indices_ptr, GL_STATIC_DRAW);
        return _elements_handle;
    }

    [[nodiscard]] static glm::uint create_attribute_buffer(const std::vector<glm::vec2>& attribute)
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

    [[nodiscard]] static glm::uint create_attribute_buffer(const std::vector<glm::vec3>& attribute)
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

    struct raycast_data {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    [[nodiscard]] static raycast_data camera_center_ray(const glm::mat4& view)
    {
        glm::mat4 _inverse_view = glm::inverse(view);
        glm::vec3 _origin = glm::vec3(_inverse_view * glm::vec4(0, 0, 0, 1));
        glm::vec3 _direction = glm::normalize(glm::vec3(_inverse_view * glm::vec4(0, 0, -1, 0)));
        return { _origin, _direction };
    }

    [[nodiscard]] static bool raycast_triangle(
        const raycast_data& raycast,
        const glm::vec3& vertex_position_a,
        const glm::vec3& vertex_position_b,
        const glm::vec3& vertex_position_c,
        glm::vec3& collision_position)
    {
        const glm::float32 _eps = 1e-7f;
        const glm::vec3 _ab = vertex_position_b - vertex_position_a;
        const glm::vec3 _ac = vertex_position_c - vertex_position_a;

        const glm::vec3 _p = glm::cross(raycast.direction, _ac);
        const glm::float32 _det = glm::dot(_ab, _p);
        if (_det < _eps) {
            return false; // we cull backfaces
        }

        const glm::float32 _inv_det = 1.f / _det;
        const glm::vec3 _s = raycast.origin - vertex_position_a;
        collision_position.y = glm::dot(_s, _p) * _inv_det;
        if (collision_position.y < 0.f || collision_position.y > 1.f) {
            return false;
        }

        const glm::vec3 _q = glm::cross(_s, _ab);
        collision_position.z = glm::dot(raycast.direction, _q) * _inv_det;
        if (collision_position.z < 0.f || collision_position.x + collision_position.z > 1.f) {
            return false;
        }

        collision_position.x = glm::dot(_ac, _q) * _inv_det;
        return collision_position.x >= 0.f;
    }

    [[nodiscard]] static glm::vec2 lerp_uv(
        const glm::vec2& vertex_texcoord_a,
        const glm::vec2& vertex_texcoord_b,
        const glm::vec2& vertex_texcoord_c,
        const glm::float32 lerp_texcoord_u,
        const glm::float32 lerp_texcoord_v)
    {
        const glm::float32 _w = 1.0f - lerp_texcoord_u - lerp_texcoord_v;
        return vertex_texcoord_a * _w + vertex_texcoord_b * lerp_texcoord_u + vertex_texcoord_c * lerp_texcoord_v;
    }

}

viewport::viewport(viewport&& other)
{
    *this = std::move(other);
}

viewport& viewport::operator=(viewport&& other)
{    
    if (_is_owning) {
        LUCARIA_RUNTIME_ERROR("Object already owning resources")
    }
    _is_owning = true;
    _computed_framebuffer_size = other._computed_framebuffer_size;
    _size = other._size;
    _positions = other._positions;
    _texcoords = other._texcoords;
    _indices = other._indices;
    _array_handle = other._array_handle;
    _elements_handle = other._elements_handle;
    _positions_handle = other._positions_handle;
    _texcoords_handle = other._texcoords_handle;
    other._is_owning = false;
    return *this;
}

viewport::~viewport()
{
    if (_is_owning) {
        glDeleteVertexArrays(1, &_array_handle);
        glDeleteBuffers(1, &_elements_handle);
        glDeleteBuffers(1, &_positions_handle);
        glDeleteBuffers(1, &_texcoords_handle);
    }
}

viewport::viewport(const geometry& from, const glm::uvec2& size_pixels)
{
    _computed_framebuffer_size = size_pixels;

    _positions = from.data.positions;
    _texcoords = from.data.texcoords;
    _indices = from.data.indices;

    _size = 3 * static_cast<glm::uint>(from.data.indices.size());
    _array_handle = create_vertex_array();
    _elements_handle = create_elements_buffer(from.data.indices);
    _positions_handle = create_attribute_buffer(from.data.positions);
    _texcoords_handle = create_attribute_buffer(invert_texcoords(from.data.texcoords));

    _is_owning = true;
}

viewport::viewport(const geometry& from, const glm::float32 pixels_per_meter)
{
    _computed_framebuffer_size = compute_size_planar_fast(
        from.data.positions,
        from.data.indices,
        pixels_per_meter);

    _positions = from.data.positions;
    _texcoords = from.data.texcoords;
    _indices = from.data.indices;

    _size = 3 * static_cast<glm::uint>(from.data.indices.size());
    _array_handle = create_vertex_array();
    _elements_handle = create_elements_buffer(from.data.indices);
    _positions_handle = create_attribute_buffer(from.data.positions);
    _texcoords_handle = create_attribute_buffer(invert_texcoords(from.data.texcoords));

    _is_owning = true;
}

std::optional<glm::vec2> viewport::raycast(const glm::mat4& view)
{
    const raycast_data _raycast = camera_center_ray(view);

    bool _has_hit = false;
    glm::float32 _best_distance = std::numeric_limits<glm::float32>::infinity();
    glm::vec2 _best_uv = glm::vec2(0);

    for (const glm::uvec3& _triangle : _indices) {
        const glm::vec3& _vertex_a = _positions[_triangle.x];
        const glm::vec3& _vertex_b = _positions[_triangle.y];
        const glm::vec3& _vertex_c = _positions[_triangle.z];

        glm::vec3 _collision_position;
        if (!raycast_triangle(_raycast, _vertex_a, _vertex_b, _vertex_c, _collision_position)) {
            continue;
        }

        if (_collision_position.x < _best_distance) {
            const glm::vec2& _texcoord_a = _texcoords[_triangle.x];
            const glm::vec2& _texcoord_b = _texcoords[_triangle.y];
            const glm::vec2& _texcoord_c = _texcoords[_triangle.z];

            _best_distance = _collision_position.x;
            _best_uv = lerp_uv(_texcoord_a, _texcoord_b, _texcoord_c, _collision_position.y, _collision_position.z);
            _has_hit = true;
        }
    }

    if (!_has_hit) {
        return std::nullopt;
    }
    return _best_uv;
}

glm::uvec2 viewport::get_computed_screen_size() const
{
    return _computed_framebuffer_size;
}

glm::uint viewport::get_size() const
{
    return _size;
}

glm::uint viewport::get_array_handle() const
{
    return _array_handle;
}

glm::uint viewport::get_elements_handle() const
{
    return _elements_handle;
}

glm::uint viewport::get_positions_handle() const
{
    return _positions_handle;
}

glm::uint viewport::get_texcoords_handle() const
{
    return _texcoords_handle;
}

fetched<viewport> fetch_viewport(const std::filesystem::path& data_path, const glm::uvec2& size_pixels)
{
    std::shared_ptr<std::promise<geometry>> _geometry_promise = std::make_shared<std::promise<geometry>>();

    detail::fetch_bytes(data_path, [_geometry_promise](const std::vector<char>& _data_bytes) {
        geometry _geometry(_data_bytes);
        _geometry_promise->set_value(std::move(_geometry));
    });

    // create viewport on main thread
    return fetched<viewport>(_geometry_promise->get_future(), [size_pixels](const geometry& _from) {
        return viewport(_from, size_pixels);
    });
}

fetched<viewport> fetch_viewport(const std::filesystem::path& data_path, const glm::float32 pixels_per_meter)
{
    std::shared_ptr<std::promise<geometry>> _geometry_promise = std::make_shared<std::promise<geometry>>();

    detail::fetch_bytes(data_path, [_geometry_promise](const std::vector<char>& _data_bytes) {
        geometry _geometry(_data_bytes);
        _geometry_promise->set_value(std::move(_geometry));
    });

    // create viewport on main thread
    return fetched<viewport>(_geometry_promise->get_future(), [pixels_per_meter](const geometry& _from) {
        return viewport(_from, pixels_per_meter);
    });
}

}