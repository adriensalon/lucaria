#include <memory>

#include <glm/gtc/type_ptr.hpp>

#include <lucaria/core/math.hpp>

namespace lucaria {
namespace {

    template <typename from_t, typename to_t>
    to_t& reinterpret_impl(from_t& from)
    {
        return *(reinterpret_cast<to_t*>(std::addressof(from)));
    }

    template <typename from_t, typename to_t>
    const to_t& reinterpret_impl(const from_t& from)
    {
        return *(reinterpret_cast<const to_t*>(std::addressof(from)));
    }

}

namespace detail {

    // to glm

    glm::mat4 convert(const btTransform& transform)
    {
        glm::mat4 _matrix;
        const btMatrix3x3& _basis = transform.getBasis();
        for (glm::uint _r = 0; _r < 3; ++_r) {
            for (glm::uint _r = 0; _r < 3; ++_r) {
                _matrix[_r][_r] = _basis[_r][_r];
            }
        }
        const btVector3& origin = transform.getOrigin();
        _matrix[3][0] = origin.x();
        _matrix[3][1] = origin.y();
        _matrix[3][2] = origin.z();
        _matrix[3][3] = 1.f;
        return _matrix;
    }

    glm::mat4& reinterpret(ozz::math::Float4x4& matrix)
    {
        return reinterpret_impl<ozz::math::Float4x4, glm::mat4>(matrix);
    }

    const glm::mat4& reinterpret(const ozz::math::Float4x4& matrix)
    {
        return reinterpret_impl<ozz::math::Float4x4, glm::mat4>(matrix);
    }

    // to ozz

    ozz::math::Float4x4& reinterpret_ozz(glm::mat4& matrix)
    {
        return reinterpret_impl<glm::mat4, ozz::math::Float4x4>(matrix);
    }

    const ozz::math::Float4x4& reinterpret_ozz(const glm::mat4& matrix)
    {
        return reinterpret_impl<glm::mat4, ozz::math::Float4x4>(matrix);
    }

    // to bullet

    btTransform convert_bullet(const glm::mat4& matrix)
    {
        btMatrix3x3 _basis;
        _basis.setFromOpenGLSubMatrix(glm::value_ptr(matrix));
        btVector3 _origin(matrix[3][0], matrix[3][1], matrix[3][2]);
        return btTransform(_basis, _origin);
    }

    btVector3& reinterpret_bullet(glm::vec3& vector)
    {
        return reinterpret_impl<glm::vec3, btVector3>(vector);
    }

    const btVector3& reinterpret_bullet(const glm::vec3& vector)
    {
        return reinterpret_impl<glm::vec3, btVector3>(vector);
    }

}
}
