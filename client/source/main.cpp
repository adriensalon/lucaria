#include <glue/fetch.hpp>
#include <glue/window.hpp>
#include <scene/scene.hpp>

#include <iostream>

int main()
{
    // skybox
    // std::future<std::vector<mesh_data>> skybox_ = fetch_meshes({
    //     "assets/collision_mesh_1.bin",
    //     "assets/collision_mesh_2.bin"
    // });

    // room
    std::future<mesh_data> room_mesh = fetch_mesh("assets/room_mesh.bin");
    std::future<texture_data> room_color = fetch_texture("assets/room_color.bin");

    // controller
    std::future<std::vector<mesh_data>> controller_meshes = fetch_meshes({
        "assets/collision_mesh_1.bin",
        "assets/collision_mesh_2.bin"
    });

    const auto sr = get_samplerate();
    std::cout << "samplerate = " << std::to_string(sr) << std::endl;

    run([&]() {
        update_splash(std::chrono::seconds(3));
        update_camera({1, 1, 1, 1});
        update_controller(controller_meshes);
        // update_skybox(_skybox_textures);
        update_room(room_mesh, room_color);
    });

    return 0;
}