#include <ecs/component/transform.hpp>
#include <ecs/component/widget.hpp>
#include <ecs/system/interface.hpp>

#include <core/world.hpp>

void interface_system::collect_gui_widgets()
{
    std::cout << "GUI widget scene \n";
    each_scene([](scene_data& scene) {
        std::cout << "widget scene \n";
        // scene.components.view<widget_component>(entt::exclude<transform_component>).each([](widget_component& widget) {
        scene.components.view<widget_component>().each([](widget_component& widget) {
            std::cout << "widget \n";
            if (widget._callback) {
                widget._callback();
            }
        });
        // scene.components.view<widget_component, transform_component>().each([](widget_component& widget, transform_component& transform) {
        //     // todo
        // });
    });
}