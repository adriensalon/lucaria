
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <entt/entt.hpp>

#include <core/window.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/controller.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/speaker.hpp>
#include <ecs/component/widget.hpp>
#include <ecs/system/mixer.hpp>
#include <ecs/system/rendering.hpp>
#include <ecs/system/scripting.hpp>
#include <ecs/system/interface.hpp>

#include <game/levels/levels.hpp>

namespace detail {

fetch_container<font_ref> big_splash_font = {};
fetch_container<font_ref> small_menu_font = {};

void draw_splash_menu()
{
    const ImVec2 _display_size = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(_display_size);
    ImGuiWindowFlags _window_flags = ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoBringToFrontOnFocus |
                                        ImGuiWindowFlags_NoNavFocus |
                                        ImGuiWindowFlags_NoBackground;
    const bool _is_ready = get_fetches_completed() == get_fetches_total();
    if (ImGui::Begin("Lucaria splash", nullptr, _window_flags)) {
        
        if (big_splash_font.has_value()) {
            ImGui::PushFont(big_splash_font.value().get_font());
            ImGui::Text("Lucaria");
            ImGui::PopFont();
        }
        const std::string _text = _is_ready ? "Press any key to enter" : "Loading assets (" + std::to_string(get_fetches_completed()) + "/" + std::to_string(get_fetches_total()) + ")";
        const ImVec2 _text_size = ImGui::CalcTextSize(_text.c_str());
        const ImVec2 _text_pos = (_display_size - _text_size) / 2.f;
        ImGui::SetCursorPos(_text_pos);        
            
        if (small_menu_font.has_value()) {
            ImGui::PushFont(small_menu_font.value().get_font());
            ImGui::Text(_text.c_str());
            ImGui::PopFont();
        }
        // if (texture.has_value())
        //     ImGui::Image((ImTextureID)(texture.value().get_id()), { 500, 500 });
        ImGui::End();
    }
    if (_is_ready && get_is_audio_locked()) {
        mark_remove_level(levelID_menu_splash);
    }
}
    
}

void level_menu_splash(entt::registry& registry)
{
    detail::big_splash_font.emplace(fetch_font({ "assets/planet.bin" }, 160.f));
    detail::small_menu_font.emplace(fetch_font({ "assets/sfprolight.bin" }, 22.f));

    const entt::entity _splash_entity = registry.create();

    registry.emplace<widget_component>(_splash_entity)
        .gui(detail::draw_splash_menu);
}