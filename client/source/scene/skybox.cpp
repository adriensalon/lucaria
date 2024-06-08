
#include <core/program.hpp>

namespace detail {

const std::string skybox_vertex = "#version 300 es \n"
                                  "in vec3 vert_position; \n"
                                  "uniform mat4 uniform_projection; \n"
                                  "out vec3 frag_texcoord; \n"
                                  "void main() { \n"
                                  "    frag_texcoord = vert_position; \n"
                                  "    gl_Position = uniform_projection * vec4(vert_position, 1); \n"
                                  "};";

const std::string skybox_fragment = "#version 300 es \n"
                                    "precision mediump float; \n"
                                    "in vec3 frag_texcoord; \n"
                                    "uniform samplerCube uniform_color; \n"
                                    "out vec4 output_color; \n"
                                    "void main() { \n"
                                    "    output_color = texture(uniform_color, frag_texcoord); \n"
                                    "};";

constexpr GLuint skybox_count = 8;

const std::vector<GLfloat> skybox_positions = {
    -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f
};

const std::vector<GLuint> skybox_indices = {
    0, 1, 2, 3, 4, 5,
    6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35
};

static std::optional<mesh_ref> skybox_mesh = std::nullopt;
static std::optional<cubemap_ref> skybox_cubemap = std::nullopt;
static std::optional<program_ref> skybox_program = std::nullopt;
static bool is_skybox_setup = false;

}

/// @brief Create GPU objects for a singleton skybox
/// @param plus_x_file the texture binary path representing the +X direction
/// @param plus_y_file the texture binary path representing the +Y direction
/// @param plus_z_file the texture binary path representing the +Z direction
/// @param minus_x_file the texture binary path representing the -X direction
/// @param minus_y_file the texture binary path representing the -Y direction
/// @param minus_z_file the texture binary path representing the -Z direction
void setup_skybox(
    const std::filesystem::path& plus_x_file,
    const std::filesystem::path& plus_y_file,
    const std::filesystem::path& plus_z_file,
    const std::filesystem::path& minus_x_file,
    const std::filesystem::path& minus_y_file,
    const std::filesystem::path& minus_z_file)
{
    detail::skybox_mesh = mesh_ref(
        mesh_data {
            detail::skybox_count,
            detail::skybox_positions, {}, {}, {}, {}, {},
            detail::skybox_indices });
    detail::skybox_cubemap = cubemap_ref(
        load_texture(plus_x_file),
        load_texture(plus_y_file),
        load_texture(plus_z_file),
        load_texture(minus_x_file),
        load_texture(minus_y_file),
        load_texture(minus_z_file));
    detail::skybox_program = program_ref(
        shader_data { detail::skybox_vertex },
        shader_data { detail::skybox_fragment });
    detail::is_skybox_setup = true;
}

/// @brief Draw the GPU objects representing a singleton skybox
void draw_skybox()
{
#if DEBUG
    if (!detail::is_skybox_setup) {
        std::cerr << "Skybox must be setup before it can be drawn" << std::endl;
        std::terminate();
    }
#endif
    detail::skybox_program.value().use();
    detail::skybox_program.value().bind(detail::skybox_mesh.value(), "vert_position", mesh_attribute::position);
    detail::skybox_program.value().bind(detail::skybox_mesh.value(), "vert_texcoord", mesh_attribute::texcoord);
    detail::skybox_program.value().bind(detail::skybox_cubemap.value(), "uniform_color", 0);
    detail::skybox_program.value().draw();
}