#include <memory>

#include <glm/gtc/type_ptr.hpp>

#include <lucaria/core/math.hpp>

namespace lucaria {
namespace detail {

    // to glm

    glm::vec3 convert(const btVector3& vector)
    {
        return glm::vec3(vector.x(), vector.y(), vector.z());
    }

    glm::vec3 convert(const ozz::math::Float3& vector)
    {
        return glm::vec3(vector.x, vector.y, vector.z);
    }

    glm::mat4 convert(const btTransform& transform)
    {
        glm::mat4 _result(1.f);
        transform.getOpenGLMatrix(glm::value_ptr(_result));
        return _result;
    }

    glm::mat4 convert(const ozz::math::Float4x4& matrix)
    {
        glm::mat4 _result(1.f);
        for (int _col_index = 0; _col_index < 4; ++_col_index) {
            _result[_col_index][0] = ozz::math::GetX(matrix.cols[_col_index]);
            _result[_col_index][1] = ozz::math::GetY(matrix.cols[_col_index]);
            _result[_col_index][2] = ozz::math::GetZ(matrix.cols[_col_index]);
            _result[_col_index][3] = ozz::math::GetW(matrix.cols[_col_index]);
        }
        return _result;
    }

    // to ozz

    ozz::math::Float4x4 convert_ozz(const glm::mat4& matrix)
    {
        ozz::math::Float4x4 _result;
        for (int _col_index = 0; _col_index < 4; ++_col_index) {
            _result.cols[_col_index] = ozz::math::simd_float4::Load(
                matrix[_col_index][0],
                matrix[_col_index][1],
                matrix[_col_index][2],
                matrix[_col_index][3]);
        }
        return _result;
    }

    // to bullet

    btVector3 convert_bullet(const glm::vec3& vector)
    {
        return btVector3(vector.x, vector.y, vector.z);
    }

    btQuaternion convert_bullet(const glm::quat& vector)
    {
        return btQuaternion(vector.x, vector.y, vector.z, vector.w);
    }

    btTransform convert_bullet(const glm::mat4& matrix)
    {
        btTransform _result;
        _result.setFromOpenGLMatrix(glm::value_ptr(matrix));
        return _result;
    }

    glm::vec3 project_on_plane(const glm::vec3& vector, const glm::vec3& normal)
    {
        return vector - normal * glm::dot(normal, vector);
    }

    btVector3 project_on_plane_bullet(const btVector3& vector, const btVector3& up)
    {
        return vector - up * vector.dot(up);
    }

}
}
