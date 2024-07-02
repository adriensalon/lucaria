#include <glue/fetch.hpp>
#include <glue/audio.hpp>
#include <glue/window.hpp>
#include <scene/scene.hpp>

#include <iostream>

int main()
{
    // controller
    std::future<volume_data> collision_volume = fetch_volume({
        "assets/collision_mesh_1.bin",
        "assets/collision_mesh_2.bin"
    });

    // skybox
    // std::future<cubemap_data> skybox_cubemap = fetch_cubemap(
    //     "assets/skybox_texture_posx.bin",
    //     "assets/skybox_texture_posy.bin",
    //     "assets/skybox_texture_posz.bin",
    //     "assets/skybox_texture_negx.bin",
    //     "assets/skybox_texture_negy.bin",
    //     "assets/skybox_texture_negz.bin"
    // );

    // room
    std::future<mesh_data> room_mesh = fetch_mesh("assets/room_mesh.bin");
    std::future<texture_data> room_color = fetch_texture("assets/room_color.bin");

    // computer
    std::future<mesh_data> screen_mesh = fetch_mesh("assets/screen_mesh.bin");
    std::future<mesh_data> speakers_mesh = fetch_mesh("assets/speaker_mesh.bin");
    std::future<texture_data> speakers_texture = fetch_texture("assets/speaker_texture.bin");
    std::future<texture_data> pointer_texture = fetch_texture("assets/pointer_texture.bin");
    // music

    run([&]() {
        update_splash(std::chrono::seconds(3));
        update_camera({1, 1, 1, 1});
        update_controller(collision_volume);
        // update_skybox(skybox_cubemap);
        update_room(room_mesh, room_color);
        // update_computer();
    });

    return 0;
}