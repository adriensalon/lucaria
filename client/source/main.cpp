#include <filesystem>
#include <functional>

#include <glm/glm.hpp>

// glue
extern void run(std::function<void()> update);
extern const std::unordered_map<std::string, bool>& get_keys();
extern const std::unordered_map<int, bool>& get_buttons();
extern glm::vec2 get_screen_size();
extern float get_aspect_ratio();
extern glm::vec2 get_mouse_position();
extern glm::vec2 get_mouse_position_delta();
extern float get_time_delta();
extern void graphics_assert();
extern void audio_assert();

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
extern void setup_room(
    const std::filesystem::path& mesh_file, 
    const std::filesystem::path& texture_file);
extern void draw_skybox();
extern void draw_room();

int main()
{
    // setup_skybox(
    //     "texture/skybox_plus_x.bin",
    //     "texture/skybox_plus_y.bin",
    //     "texture/skybox_plus_z.bin",
    //     "texture/skybox_minus_x.bin",
    //     "texture/skybox_minus_y.bin",
    //     "texture/skybox_minus_z.bin");
    setup_room(
        "mesh/room.bin",
        "texture/room_color.bin");
    // setup_screen();
    // setup_speaker();

    run([&]() {
        update_controller();
        clear_camera();
        // draw_skybox();
        draw_room();
    });
    return 0;
}