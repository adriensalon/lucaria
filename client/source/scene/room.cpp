#include <core/program.hpp>

extern glm::mat4x4 get_view_projection_matrix();

namespace detail {

static std::optional<mesh_ref> room_mesh;
static std::optional<texture_ref> room_texture;
static std::optional<program_ref> room_program;

const std::string room_vertex = "#version 300 es \n"
                                       "in vec3 vert_position; \n"
                                       "in vec2 vert_texcoord; \n"
                                       "uniform mat4 uniform_view; \n"
                                       "out vec2 frag_texcoord; \n"
                                       "void main() { \n"
                                       "    frag_texcoord = vert_texcoord; \n"
                                       "    gl_Position = uniform_view * vec4(vert_position, 1); \n"
                                       "};";

const std::string room_fragment = "#version 300 es \n"
                                         "precision mediump float; \n"
                                         "in vec2 frag_texcoord; \n"
                                         "uniform sampler2D uniform_color; \n"
                                         "out vec4 output_color; \n"
                                         "void main() { \n"
                                         "    output_color = texture(uniform_color, frag_texcoord); \n"
                                         "};";

}

/// @brief
/// @param mesh_file
/// @param texture_file
void setup_room(
    const std::filesystem::path& mesh_file,
    const std::filesystem::path& texture_file)
{
    detail::room_mesh = mesh_ref(load_mesh(mesh_file));
    detail::room_texture = texture_ref(load_texture(texture_file));
    detail::room_program = program_ref(
        shader_data { detail::room_vertex },
        shader_data { detail::room_fragment });
}

/// @brief 
/// @param camera_position 
/// @param camera_rotation 
void draw_room()
{
    detail::room_program.value().use();
    detail::room_program.value().bind("vert_position", detail::room_mesh.value(), mesh_attribute::position);
    detail::room_program.value().bind("vert_texcoord", detail::room_mesh.value(), mesh_attribute::texcoord);
    detail::room_program.value().bind("uniform_color", detail::room_texture.value(), 0);
    detail::room_program.value().bind("uniform_view", get_view_projection_matrix());
    detail::room_program.value().draw();
}