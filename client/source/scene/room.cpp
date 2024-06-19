#include <iostream>
#include <imgui.h>

#include <core/program.hpp>
#include <glue/fetch.hpp>

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

void setup_room(const mesh_data& mesh, const texture_data& texture)
{
    room = room_ref {
        mesh_ref(mesh),
        texture_ref(texture),
        program_ref(
            shader_data { room_vertex },
            shader_data { room_fragment })
    };
}

bool setup_room_async(std::future<mesh_data>& mesh, std::future<texture_data>& texture)
{
    if (is_future_ready(mesh) && is_future_ready(texture)) {
        mesh_data _room_mesh_data = mesh.get();
        texture_data _room_color_data = texture.get();
        setup_room(_room_mesh_data, _room_color_data);
        return true;
    }
    return false;
}

void draw_room()
{
#if LUCARIA_DEBUG
    if (!room.has_value()) {
        std::cout << "Forgot to call setup_room() before draw_room()" << std::endl;
        std::terminate();
    }
#endif
    room_ref& _room = room.value();
    _room.program.use();
    _room.program.bind("vert_position", _room.mesh, mesh_attribute::position);
    _room.program.bind("vert_texcoord", _room.mesh, mesh_attribute::texcoord);
    _room.program.bind("uniform_color", _room.color, 0);
    _room.program.bind("uniform_view", get_view_projection_matrix());
    _room.program.draw();
}

}

/// @brief 
/// @param mesh 
/// @param texture 
void update_room(std::future<mesh_data>& mesh, std::future<texture_data>& texture)
{
    static bool _is_room_setup = false;
    if (!_is_room_setup) {
        _is_room_setup = detail::setup_room_async(mesh, texture);
        if (ImGui::Begin("Room")) {
            ImGui::Text("Loading room...");
            ImGui::End();
        }
    } else {
        detail::draw_room();
        if (ImGui::Begin("Room")) {
            ImGui::Text("Room loaded");
            ImGui::End();
        }
    }
}