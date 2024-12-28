#include <chrono>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <entt/entt.hpp>
#include <imgui.h>
#include <imgui_internal.h>

#include <core/window.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/speaker.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/system/mixer.hpp>
#include <ecs/system/rendering.hpp>

#include <game/actor/menu_splash.hpp>
#include <game/scene/user_game.hpp>
#include <game/scene/zone_flight.hpp>

namespace detail {

const ImVec4 white_text_color = { 1.f, 1.f, 1.f, 0.98f };
constexpr ImGuiWindowFlags fullscreen_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

}

menu_splash_actor::menu_splash_actor(scene_data& scene)
{
    big_splash_font.emplace(fetch_font({ "assets/font/font_LYj3.bin" }, 160.f));
    small_menu_font.emplace(fetch_font({ "assets/font/font_xVRp.bin" }, 22.f));
    background_splash_texture.emplace(fetch_texture("assets/image/image_0cSX.bin"));
}

void menu_splash_actor::update()
{
    if (is_splash_resources_fetched) {
        static bool _has_added_first_level = false;
        if (!_has_added_first_level || (_has_added_first_level && get_fetches_waiting() > 0)) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
            ImGui::PushStyleColor(ImGuiCol_Text, detail::white_text_color);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, detail::white_text_color);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.f, 1.f, 1.f, 0.f));
            const ImVec2 _display_size = ImGui::GetIO().DisplaySize;
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(_display_size);
            if (ImGui::Begin("Lucaria splash", nullptr, detail::fullscreen_window_flags)) {
                const float _initial_weight = splash_background_fadein.compute_weight(splash_resources_fetched_cursor);
                const ImVec4 _tint = { 1.f, 1.f, 1.f, _initial_weight };
                ImGui::Image(reinterpret_cast<ImTextureID>(background_splash_texture.value().get_id()), _display_size, { 0.001f, 0.001f }, { 0.999f, 0.999f }, _tint, { 1.f, 1.f, 1.f, 1.f });
                ImGui::PushFont(big_splash_font.value().get_font());
                const ImVec2 _text_size = ImGui::CalcTextSize("lucaria");
                const ImVec2 _text_pos = (_display_size - _text_size) / 2.f - ImVec2(0.f, 100.f);
                ImGui::SetCursorPos(_text_pos);
                ImGui::Text("lucaria");
                ImGui::PopFont();
                const float _weight = small_text_oscillate.compute_weight(splash_resources_fetched_cursor);
                const std::string _text = (get_fetches_waiting() == 0) ? "Press any key to enter" : "Loading assets (" + std::to_string(get_fetches_waiting()) + " files remaining)";
                ImGui::PushFont(small_menu_font.value().get_font());
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, _weight));
                const ImVec2 _text_size2 = ImGui::CalcTextSize(_text.c_str());
                const ImVec2 _text_pos2 = (_display_size - _text_size2) / 2.f + ImVec2(0.f, 100.f);
                ImGui::SetCursorPos(_text_pos2);
                ImGui::Text(_text.c_str());
                ImGui::PopStyleColor();
                ImGui::PopFont();
                ImGui::End();
            }
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar(4);
            splash_resources_fetched_cursor += 1.f / 60.f; //get_time_delta();
        }
        if (get_is_audio_locked() && get_is_mouse_locked()) {
            if (!_has_added_first_level) {
                make_scene<user_game_scene>();
                make_scene<zone_flight_scene>();
                _has_added_first_level = true;
            }
        }
    } else if (big_splash_font.has_value() && small_menu_font.has_value() && background_splash_texture.has_value()) {
        is_splash_resources_fetched = true;
    }
}