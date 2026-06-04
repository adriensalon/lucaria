#pragma once

#include <lucaria/core/object_geometry.hpp>
#include <lucaria/core/utils_owning.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend_pspgu.hpp>
#endif

#include <lucaria/core/context_serialize.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct system_rendering;
    struct manager_assets;

    enum struct object_mesh_attribute {
        position,
        color,
        normal,
        tangent,
        bitangent,
        texcoord,
        bones,
        weights
    };

    enum struct object_mesh_origin {
        path,
        data
    };

    struct object_mesh {
        object_mesh() = default;
        object_mesh(const object_mesh& other) = delete;
        object_mesh& operator=(const object_mesh& other) = delete;
        object_mesh(object_mesh&& other) = default;
        object_mesh& operator=(object_mesh&& other) = default;
        ~object_mesh();

        object_mesh(const object_geometry& geometry);

        object_mesh_origin origin;
        std::filesystem::path origin_path;
        std::vector<float32x4x4> invposes;
        uint32 size;

        void save(storage_save_context& context) const
        {
            context.field("origin", origin);
            if (origin == object_mesh_origin::path) {
                context.field("origin_path", origin_path);
            }
        }

        void load(storage_load_context& context)
        {
            context.field("origin", origin);
            if (origin == object_mesh_origin::path) {
                context.field("origin_path", origin_path);
                const std::filesystem::path _path = origin_path;
                context.fetch(_path, [this, _path](const std::vector<char>& bytes) {
                    object_geometry _geometry(bytes);
                    *this = object_mesh(_geometry);
                    origin_path = _path;
                });
            }
        }


#if defined(LUCARIA_BACKEND_OPENGL)
        flag_owning ownership = {};
        GLuint array_id = 0;
        GLuint elements_id = 0;
        std::unordered_map<object_mesh_attribute, GLuint> attribute_ids = {};
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        flag_owning ownership = {};
        void* vertices = nullptr; // aligned CPU memory
        uint32 vertex_count = 0;
        int vertex_type = 0; // GU_VERTEX_32BITF | GU_TEXTURE_32BITF...
        int primitive = GU_TRIANGLES;
#endif
    };

    struct object_mesh_line {
        LUCARIA_DELETE_DEFAULT(object_mesh_line)
        object_mesh_line(const object_mesh_line& other) = delete;
        object_mesh_line& operator=(const object_mesh_line& other) = delete;
        object_mesh_line(object_mesh_line&& other) = default;
        object_mesh_line& operator=(object_mesh_line&& other) = default;
        ~object_mesh_line();

        object_mesh_line(const object_geometry& from);
        object_mesh_line(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices);
        void update(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices);

        uint32 size;

#if defined(LUCARIA_BACKEND_OPENGL)
        flag_owning ownership = {};
        GLuint array_handle = 0;
        GLuint elements_handle = 0;
        GLuint positions_handle = 0;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#endif
    };
}
}
