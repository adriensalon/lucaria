#include <glm/gtc/matrix_inverse.hpp>

#include <lucaria/core/opengl.hpp>
#include <lucaria/core/viewport.hpp>

namespace lucaria {
namespace {

    [[nodiscard]] static inline std::vector<glm::vec2> invert_texcoords(const std::vector<glm::vec2>& texcoords)
    {
        std::vector<glm::vec2> _inverted;
        _inverted.reserve(texcoords.size());
        for (const glm::vec2& _texcoord : texcoords) {
            _inverted.push_back({ _texcoord.x, 1.0f - _texcoord.y });
        }
        return _inverted;
    }

    // Branchless Frisvad ONB with the singular pole at N.y == -1
    // N must be (approximately) normalized.
    static inline void make_onb_frisvad_branchless_poleY(const glm::vec3& N, glm::vec3& U, glm::vec3& V)
    {
        // Permute so Y becomes Z in the formula
        const glm::vec3 Np(N.x, N.z, N.y); // (x, z, y)

        // Duff/Frisvad branchless form (normally singular at Np.z == -1)
        glm::float32 sign = std::copysign(1.0f, Np.z);
        glm::float32 denom = sign + Np.z;

        // (Optional) safety clamp to avoid Inf at exactly -1 while keeping it branchless-ish
        // Comment this out if you truly want no conditionals at all.
        denom = (std::fabs(denom) < 1e-12f) ? ((denom < 0.0f) ? -1e-12f : 1e-12f) : denom;

        const glm::float32 a = -1.0f / denom;
        const glm::float32 b = Np.x * Np.y * a;

        // Tangent/bitangent in permuted space
        const glm::vec3 Up(1.0f + sign * Np.x * Np.x * a, sign * b, -sign * Np.x);
        const glm::vec3 Vp(b, sign + Np.y * Np.y * a, -Np.y);

        // Un-permute back (swap Y↔Z)
        U = glm::vec3(Up.x, Up.z, Up.y);
        V = glm::vec3(Vp.x, Vp.z, Vp.y);

        // Optional: normalize in case N isn't unit, and enforce right-handedness
        U = glm::normalize(U);
        V = glm::normalize(V);
        if (glm::dot(glm::cross(U, V), N) < 0.0f) {
            V = -V;
        }

        // Optional: deterministic sign to avoid 180° flips across frames
        // if (U.x < 0) { U = -U; V = -V; }
    }

    [[nodiscard]] static inline glm::uvec2 compute_size_planar_fast(
        const std::vector<glm::vec3>& positions,
        const std::vector<glm::uvec3>& indices,
        const glm::float32 pixels_per_meter,
        const glm::uint min_side = 16,
        const glm::uint max_side = 4096)
    {
        // 1) Find a face normal (assumes coplanar faces)
        glm::vec3 N(0, 0, 1);
        for (auto& tri : indices) {
            const glm::vec3 a = positions[tri.x];
            const glm::vec3 b = positions[tri.y];
            const glm::vec3 c = positions[tri.z];
            const glm::vec3 n = glm::cross(b - a, c - a);
            if (glm::length(n) > 1e-10f) { // note: threshold must be sqrt of before
                N = glm::normalize(n);
                break;
            }
        }

        // 2) Robust in-plane basis (orientation-agnostic)
        glm::vec3 U, V;
        make_onb_frisvad_branchless_poleY(N, U, V);

        // 3) Project to plane and compute in-plane AABB
        glm::float32 minU = FLT_MAX, maxU = -FLT_MAX, minV = FLT_MAX, maxV = -FLT_MAX;
        for (auto& p : positions) {
            const glm::float32 u = glm::dot(p, U);
            const glm::float32 v = glm::dot(p, V);
            minU = std::min(minU, u);
            maxU = std::max(maxU, u);
            minV = std::min(minV, v);
            maxV = std::max(maxV, v);
        }
        const glm::float32 width_m = std::max(0.0f, maxU - minU);
        const glm::float32 height_m = std::max(0.0f, maxV - minV);

        // 4) Convert to pixels (+ clamps)
        glm::uint W = static_cast<glm::uint>(std::lround(pixels_per_meter * width_m));
        glm::uint H = static_cast<glm::uint>(std::lround(pixels_per_meter * height_m));
        W = glm::clamp(W, min_side, max_side);
        H = glm::clamp(H, min_side, max_side);
        return { W, H };
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

    struct ray_data {
        glm::vec3 o, d;
    };

    // Ray from camera center (forward through view -Z)
    static ray_data makeCameraCenterRay(const glm::mat4& view)
    {
        glm::mat4 invV = glm::inverse(view);
        glm::vec3 origin = glm::vec3(invV * glm::vec4(0, 0, 0, 1));
        glm::vec3 forward = glm::normalize(glm::vec3(invV * glm::vec4(0, 0, -1, 0)));
        return { origin, forward };
    }

    // Möller–Trumbore with barycentric output; one-sided
    static bool rayTriangle(const ray_data& r,
        const glm::vec3& a,
        const glm::vec3& b,
        const glm::vec3& c,
        float& t, float& u, float& v)
    {
        const float EPS = 1e-7f;
        glm::vec3 ab = b - a;
        glm::vec3 ac = c - a;

        glm::vec3 p = glm::cross(r.d, ac);
        float det = glm::dot(ab, p);
        if (det < EPS) // backface culling
            return false;
        float invDet = 1.0f / det;

        glm::vec3 s = r.o - a;
        u = glm::dot(s, p) * invDet;
        if (u < 0.0f || u > 1.0f)
            return false;

        glm::vec3 q = glm::cross(s, ab);
        v = glm::dot(r.d, q) * invDet;
        if (v < 0.0f || u + v > 1.0f)
            return false;

        t = glm::dot(ac, q) * invDet;
        return t >= 0.0f;
    }

    inline glm::vec2 lerpUV(const glm::vec2& uv0,
        const glm::vec2& uv1,
        const glm::vec2& uv2,
        float u, float v)
    {
        float w = 1.0f - u - v;
        return uv0 * w + uv1 * u + uv2 * v;
    }

}

viewport::viewport(viewport&& other)
{
    *this = std::move(other);
}

viewport& viewport::operator=(viewport&& other)
{
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
    ray_data ray = makeCameraCenterRay(view);

    bool hitAny = false;
    float bestT = std::numeric_limits<float>::infinity();
    glm::vec2 bestUV { 0.0f };

    for (const glm::uvec3& tri : _indices) {
        const glm::vec3& a = _positions[tri.x];
        const glm::vec3& b = _positions[tri.y];
        const glm::vec3& c = _positions[tri.z];

        float t, u, v;
        if (!rayTriangle(ray, a, b, c, t, u, v))
            continue;

        if (t < bestT) {
            const glm::vec2& uv0 = _texcoords[tri.x];
            const glm::vec2& uv1 = _texcoords[tri.y];
            const glm::vec2& uv2 = _texcoords[tri.z];

            bestT = t;
            bestUV = lerpUV(uv0, uv1, uv2, u, v);
            hitAny = true;
        }
    }

    if (!hitAny)
        return std::nullopt;
    return bestUV;
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