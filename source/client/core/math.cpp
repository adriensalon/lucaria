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
        glm::mat4 result(1.0f); // identity

        const btMatrix3x3& basis = transform.getBasis();
        // copy rotation basis
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                result[col][row] = basis[row][col];
                // glm::mat4 is column-major, btMatrix3x3 is row-major
            }
        }

        // copy translation
        const btVector3& origin = transform.getOrigin();
        result[3][0] = origin.x();
        result[3][1] = origin.y();
        result[3][2] = origin.z();

        return result;
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
