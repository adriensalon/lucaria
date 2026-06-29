#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>

#include <lucaria/core/rendering_program.hpp>

namespace lucaria {
namespace detail {

    rendering_program::~rendering_program() = default;

    rendering_program::rendering_program(const object_shader&, const object_shader&)
    {
        ownership.emplace();
    }

    void rendering_program::use() const
    {
        sceGuEnable(GU_CULL_FACE);
        sceGuFrontFace(GU_CCW);
    }

    void rendering_program::bind_attribute(const std::string&, const rendering_mesh& from, const data_vertex_attribute)
    {
        mesh = &from;
    }

    void rendering_program::bind_uniform(const std::string&, const rendering_cubemap&, const uint32) const
    {
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
    void rendering_program::bind_uniform<float32x4x4>(const std::string&, const float32x4x4&)
    {
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
        if (use_depth && depth_enabled) {
            sceGuEnable(GU_DEPTH_TEST);
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
