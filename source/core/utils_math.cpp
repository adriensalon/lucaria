#include <memory>

#include <glm/gtc/type_ptr.hpp>

#include <lucaria/core/utils_math.hpp>

namespace lucaria {
namespace detail {

    float32x3 project_on_plane(const float32x3& vector, const float32x3& normal)
    {
        return vector - normal * glm::dot(normal, vector);
    }

    float32x3 convert(const btVector3& vector)
    {
        return float32x3(vector.x(), vector.y(), vector.z());
    }

    float32x3 convert(const ozz::math::Float3& vector)
    {
        return float32x3(vector.x, vector.y, vector.z);
    }

    float32x4x4 convert(const btTransform& transform)
    {
        float32x4x4 _result(1.f);
        transform.getOpenGLMatrix(glm::value_ptr(_result));
        return _result;
    }

    float32x4x4 convert(const ozz::math::Float4x4& matrix)
    {
        float32x4x4 _result(1.f);
        for (int _col_index = 0; _col_index < 4; ++_col_index) {
            _result[_col_index][0] = ozz::math::GetX(matrix.cols[_col_index]);
            _result[_col_index][1] = ozz::math::GetY(matrix.cols[_col_index]);
            _result[_col_index][2] = ozz::math::GetZ(matrix.cols[_col_index]);
            _result[_col_index][3] = ozz::math::GetW(matrix.cols[_col_index]);
        }
        return _result;
    }

    ozz::math::Float4x4 convert_ozz(const float32x4x4& matrix)
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

    btVector3 convert_bullet(const float32x3& vector)
    {
        return btVector3(vector.x, vector.y, vector.z);
    }

    btQuaternion convert_bullet(const float32quat& vector)
    {
        return btQuaternion(vector.x, vector.y, vector.z, vector.w);
    }

    btTransform convert_bullet(const float32x4x4& matrix)
    {
        btTransform _result;
        _result.setFromOpenGLMatrix(glm::value_ptr(matrix));
        return _result;
    }

    ImVec2 convert_imgui(const float32x2& vector)
    {
        return ImVec2(vector.x, vector.y);
    }

}
}
