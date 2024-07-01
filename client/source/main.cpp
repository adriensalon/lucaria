#include <glue/fetch.hpp>
#include <glue/window.hpp>
#include <scene/scene.hpp>

int main()
{
    std::future<mesh_data> _room_mesh = fetch_mesh("assets/room_mesh.bin");
    std::future<texture_data> _room_color = fetch_texture("assets/room_color.bin");
    // std::array<std::future<texture_data>, 6> _skybox_textures = {
    //     fetch_texture("assets/room_color.bin"),
    //     fetch_texture("assets/room_color.bin"),
    //     fetch_texture("assets/room_color.bin"),
    //     fetch_texture("assets/room_color.bin"),
    //     fetch_texture("assets/room_color.bin"),
    //     fetch_texture("assets/room_color.bin")
    // };

    run([&]() {
        update_camera({1, 1, 1, 1});
        update_controller();
        // update_skybox(_skybox_textures);
        update_room(_room_mesh, _room_color);
        update_splash(std::chrono::seconds(3), _room_mesh, _room_color);
    });

    return 0;
}