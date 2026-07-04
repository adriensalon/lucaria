#include <cstring>

#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>

#include <lucaria/core/rendering_program.hpp>

namespace lucaria {
namespace detail {

    namespace {

        [[nodiscard]] ScePspFMatrix4 _to_psp_matrix(const float32x4x4& value)
        {
            ScePspFMatrix4 _matrix;
            static_assert(sizeof(_matrix) == sizeof(value));
            std::memcpy(&_matrix, &value, sizeof(_matrix));
            return _matrix;
        }

    }

    rendering_program::~rendering_program() = default;

    rendering_program::rendering_program(const object_shader&, const object_shader&)
    {
        ownership.emplace();
    }

    void rendering_program::use() const
    {
        sceGuDisable(GU_CULL_FACE);
        sceGuDisable(GU_BLEND);
        sceGuFrontFace(GU_CCW);
        sceGuColor(0xffffffff);
        texture = nullptr;
        texture_enabled = false;
        transform_bound = false;
    }

    void rendering_program::bind_attribute(const std::string&, const rendering_mesh& from, const data_vertex_attribute)
    {
        mesh = &from;
    }

    void rendering_program::bind_uniform(const std::string&, const rendering_cubemap& from, const uint32) const
    {
        for (const std::optional<rendering_texture>& _face : from.faces) {
            if (_face.has_value()) {
                texture = &_face.value();
                texture_enabled = true;
                return;
            }
        }
    }

    void rendering_program::bind_uniform(const std::string&, const rendering_texture& from, const uint32) const
    {
        texture = &from;
        texture_enabled = true;
    }

    template <>
    void rendering_program::bind_uniform<int32>(const std::string&, const int32&)
    {
    }

    template <>
    void rendering_program::bind_uniform<float32>(const std::string&, const float32&)
    {
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32>>(const std::string&, const std::vector<float32>&)
    {
    }

    template <>
    void rendering_program::bind_uniform<float32x2>(const std::string&, const float32x2&)
    {
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32x2>>(const std::string&, const std::vector<float32x2>&)
    {
    }

    template <>
    void rendering_program::bind_uniform<float32x3>(const std::string&, const float32x3&)
    {
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32x3>>(const std::string&, const std::vector<float32x3>&)
    {
    }

    template <>
    void rendering_program::bind_uniform<float32x4>(const std::string&, const float32x4&)
    {
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32x4>>(const std::string&, const std::vector<float32x4>&)
    {
    }

    template <>
    void rendering_program::bind_uniform<float32x4x4>(const std::string&, const float32x4x4& value)
    {
        ScePspFMatrix4 _matrix = _to_psp_matrix(value);
        sceGumMatrixMode(GU_PROJECTION);
        sceGumLoadMatrix(&_matrix);
        sceGumMatrixMode(GU_VIEW);
        sceGumLoadIdentity();
        sceGumMatrixMode(GU_MODEL);
        sceGumLoadIdentity();
        transform_bound = true;
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32x4x4>>(const std::string&, const std::vector<float32x4x4>&)
    {
    }

    template <>
    void rendering_program::bind_uniform<ozz::vector<ozz::math::Float4x4>>(const std::string&, const ozz::vector<ozz::math::Float4x4>&)
    {
    }

    void rendering_program::draw(const bool use_depth) const
    {
        if (mesh == nullptr || mesh->vertex_bytes.empty() || mesh->elements_16.empty()) {
            return;
        }
        if (!transform_bound) {
            sceGumMatrixMode(GU_PROJECTION);
            sceGumLoadIdentity();
            sceGumMatrixMode(GU_VIEW);
            sceGumLoadIdentity();
            sceGumMatrixMode(GU_MODEL);
            sceGumLoadIdentity();
        }
        if (use_depth && depth_enabled) {
            sceGuEnable(GU_DEPTH_TEST);
            sceGuDepthFunc(GU_GEQUAL);
            sceGuDepthMask(GU_FALSE);
        } else {
            sceGuDisable(GU_DEPTH_TEST);
            sceGuDepthMask(GU_TRUE);
        }
        if (texture_enabled && texture != nullptr && texture->pixels != nullptr) {
            sceGuEnable(GU_TEXTURE_2D);
            sceGuTexMode(texture->psm, 0, 0, GU_FALSE);
            sceGuTexImage(0, texture->texture_capacity.x, texture->texture_capacity.y, texture->tbw, texture->pixels);
            sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
            sceGuTexFilter(GU_LINEAR, GU_LINEAR);
            sceGuTexWrap(GU_CLAMP, GU_CLAMP);
            sceGuTexScale(1.f, 1.f);
            sceGuTexOffset(0.f, 0.f);
        } else {
            sceGuDisable(GU_TEXTURE_2D);
        }
        sceGumDrawArray(
            GU_TRIANGLES,
            mesh->vertex_format | GU_INDEX_16BIT | GU_TRANSFORM_3D,
            mesh->size,
            mesh->elements_16.data(),
            mesh->vertex_bytes.data());
    }

#if defined(LUCARIA_DEBUG)
    void rendering_program::bind_guizmo(const std::string&, const rendering_mesh_line&)
    {
    }

    void rendering_program::draw_guizmo() const
    {
    }
#endif

    void rendering_program::viewport(const uint32x2 size)
    {
        sceGuOffset(2048 - static_cast<int>(size.x / 2), 2048 - static_cast<int>(size.y / 2));
        sceGuViewport(2048, 2048, static_cast<int>(size.x), static_cast<int>(size.y));
        sceGuDepthRange(65535, 0);
        sceGuScissor(0, 0, static_cast<int>(size.x), static_cast<int>(size.y));
    }

    void rendering_program::clear(const bool clear_depth)
    {
        sceGuClearColor(0xffffffff);
        sceGuClearDepth(0);
        sceGuClear(clear_depth ? GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT : GU_COLOR_BUFFER_BIT);
    }

}
}
