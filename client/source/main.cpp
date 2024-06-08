#include <glue/audio.hpp>
#include <glue/graphics.hpp>
#include <glue/window.hpp>

#include <core/program.hpp>
#include <core/texture.hpp>

int main()
{
    texture_ref _tex3(load_texture("texture/3.bin"));
    program_ref _prg(load_shader("shader/unlit_vert.bin"), load_shader("shader/unlit_frag.bin"));

    lucaria::set_perspective(60.f, 0.1f, 100.f);
    lucaria::load_unlit_shader();
    // lucaria::load_model_gltf("model.glb");
    lucaria::run([]() {
        const glm::vec2 _mouse_delta = lucaria::get_mouse_position_delta();
        const float _time_delta = lucaria::get_time_delta();
        lucaria::rotate_camera(_mouse_delta * _time_delta);
        lucaria::clear();
        
        lucaria::draw();

    });
    return 0;
}