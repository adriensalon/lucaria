#include <filesystem>
#include <functional>
#include <iostream>

#include <glm/glm.hpp>

#include <glue/fetch.hpp>
#include <core/mesh.hpp>
#include <core/texture.hpp>

// glue
extern void run(std::function<void()> update);
extern const std::unordered_map<std::string, bool>& get_keys();
extern const std::unordered_map<int, bool>& get_buttons();
extern glm::vec2 get_screen_size();
extern float get_aspect_ratio();
extern glm::vec2 get_mouse_position();
extern glm::vec2 get_mouse_position_delta();
extern float get_time_delta();

//scene
extern void clear_camera(
    const glm::vec4& color = { 0, 0, 0, 1 }, 
    const bool depth = true);
extern void update_controller();
extern void setup_skybox(
    const std::filesystem::path& plus_x_file, 
    const std::filesystem::path& plus_y_file, 
    const std::filesystem::path& plus_z_file, 
    const std::filesystem::path& minus_x_file, 
    const std::filesystem::path& minus_y_file, 
    const std::filesystem::path& minus_z_file);
extern void draw_skybox();

// room
extern void update_room(std::future<mesh_data>&, std::future<texture_data>&);

int main()
{
    std::future<mesh_data> _room_mesh = fetch_mesh("assets/room_mesh.bin");
    std::future<texture_data> _room_color = fetch_texture("assets/room_color.bin");


    // setup_room("room_mesh.bin", "room_color.bin");
    // setup_skybox(
    //     "texture/skybox_plus_x.bin",
    //     "texture/skybox_plus_y.bin",
    //     "texture/skybox_plus_z.bin",
    //     "texture/skybox_minus_x.bin",
    //     "texture/skybox_minus_y.bin",
    //     "texture/skybox_minus_z.bin");
    // setup_screen();
    // setup_speaker();

    run([&]() {

        update_controller();
        clear_camera();
        // draw_skybox();


        update_room(_room_mesh, _room_color);


    });
    return 0;
}