#include <iostream>

#include <core/program.hpp>

extern glm::mat4x4 get_view_projection_matrix();

namespace detail {

struct room_ref {
    mesh_ref mesh;
    texture_ref color;
    program_ref program;
};

static std::optional<room_ref> room = std::nullopt;

const std::string room_vertex = "#version 300 es \n"
                                       "in vec3 vert_position; \n"
                                       "in vec2 vert_texcoord; \n"
                                       "uniform mat4 uniform_view; \n"
                                       "out vec2 frag_texcoord; \n"
                                       "void main() { \n"
                                       "    frag_texcoord = vert_texcoord; \n"
                                       "    gl_Position = uniform_view * vec4(vert_position, 1); \n"
                                       "}";

const std::string room_fragment = "#version 300 es \n"
                                         "precision mediump float; \n"
                                         "in vec2 frag_texcoord; \n"
                                         "uniform sampler2D uniform_color; \n"
                                         "out vec4 output_color; \n"
                                         "void main() { \n"
                                         "    output_color = texture(uniform_color, frag_texcoord); \n"
                                         "}";

}

/// @brief Setups the room singleton instance once for rendering
/// @param mesh_file is the path to the binary mesh to use
/// @param texture_file is the path to the binary texture to use
void setup_room(
    const std::filesystem::path& mesh_file,
    const std::filesystem::path& texture_file)
{
    detail::room = detail::room_ref {
        mesh_ref(load_mesh(mesh_file)),
        texture_ref(load_texture(texture_file)),
        program_ref(
            shader_data { detail::room_vertex },
            shader_data { detail::room_fragment })
    };
}

/// @brief Renders the room singleton instance once per frame
void draw_room()
{
#if LUCARIA_DEBUG
    if (!detail::room.has_value()) {
        std::cout << "Forgot to call setup_room() before draw_room()" << std::endl;
        std::terminate();
    }
#endif
    detail::room_ref& _room = detail::room.value();
    _room.program.use();
    _room.program.bind("vert_position", _room.mesh, mesh_attribute::position);
    _room.program.bind("vert_texcoord", _room.mesh, mesh_attribute::texcoord);
    _room.program.bind("uniform_color", _room.color, 0);
    _room.program.bind("uniform_view", get_view_projection_matrix());
    _room.program.draw();
}