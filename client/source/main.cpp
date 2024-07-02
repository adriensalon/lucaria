#include <glue/fetch.hpp>
#include <glue/audio.hpp>
#include <glue/window.hpp>
#include <scene/scene.hpp>

#include <iostream>

int main()
{
    // skybox
    // std::future<cubemap_data> skybox_cubemap = fetch_cubemap(
    //     "assets/collision_mesh_1.bin",
    //     "assets/collision_mesh_2.bin"
    // );

    // room
    std::future<mesh_data> room_mesh = fetch_mesh("assets/room_mesh.bin");
    std::future<texture_data> room_color = fetch_texture("assets/room_color.bin");

    // controller
    std::future<volume_data> collision_volume = fetch_volume({
        "assets/collision_mesh_1.bin",
        "assets/collision_mesh_2.bin"
    });

    const auto sr = get_samplerate();
    std::cout << "samplerate = " << std::to_string(sr) << std::endl;

    run([&]() {
        update_splash(std::chrono::seconds(3));
        update_camera({1, 1, 1, 1});
        update_controller(collision_volume);
        // update_skybox(skybox_cubemap);
        update_room(room_mesh, room_color);
    });

    return 0;
}