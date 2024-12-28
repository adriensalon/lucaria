#include <chrono>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <entt/entt.hpp>
#include <imgui.h>
#include <imgui_internal.h>

#include <core/weight.hpp>
#include <core/window.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/speaker.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/component/widget.hpp>
#include <ecs/system/interface.hpp>
#include <ecs/system/mixer.hpp>
#include <ecs/system/rendering.hpp>

#include <game/scene/menu_splash.hpp>
#include <game/scene/user_player.hpp>
#include <game/scene/zone_flight.hpp>

namespace detail {

fetch_container<font_ref> big_splash_font = {};
fetch_container<font_ref> small_menu_font = {};
fetch_container<texture_ref> background_splash_texture = {};

const ImVec4 white_text_color = { 1.f, 1.f, 1.f, 0.98f };

constexpr ImGuiWindowFlags fullscreen_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

static bool is_splash_resources_fetched = false;
// static bool is_next_level_fetched = false;
static glm::float32 splash_resources_fetched_cursor = 0.f;
static fadein_weight splash_background_fadein = { 3.f };
static oscillate_weight small_text_oscillate = { 2.f };

static void draw_background(const ImVec2& display_size)
{
    const glm::float32 _weight = splash_background_fadein.compute_weight(splash_resources_fetched_cursor);
    const ImVec4 _tint = { 1.f, 1.f, 1.f, _weight };
    ImGui::Image(reinterpret_cast<ImTextureID>(background_splash_texture.value().get_id()), display_size, { 0.001f, 0.001f }, { 0.999f, 0.999f }, _tint, { 1.f, 1.f, 1.f, 1.f });
}

static void draw_title(const ImVec2& display_size)
{
    const std::string _text = "lucaria";
    ImGui::PushFont(big_splash_font.value().get_font());
    const ImVec2 _text_size = ImGui::CalcTextSize(_text.c_str());
    const ImVec2 _text_pos = (display_size - _text_size) / 2.f - ImVec2(0.f, 100.f);
    ImGui::SetCursorPos(_text_pos);
    ImGui::Text("Lucaria");
    ImGui::PopFont();
}

static void draw_text(const ImVec2& display_size)
{
    const glm::float32 _weight = small_text_oscillate.compute_weight(splash_resources_fetched_cursor);
    const std::string _text = (get_fetches_waiting() == 0) ? "Press any key to enter" : "Loading assets (" + std::to_string(get_fetches_waiting()) + " files remaining)";
    ImGui::PushFont(small_menu_font.value().get_font());
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, _weight));
    const ImVec2 _text_size = ImGui::CalcTextSize(_text.c_str());
    const ImVec2 _text_pos = (display_size - _text_size) / 2.f + ImVec2(0.f, 100.f);
    ImGui::SetCursorPos(_text_pos);
    ImGui::Text(_text.c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();
}

static void draw_splash_menu()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_Text, white_text_color);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, white_text_color);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.f, 1.f, 1.f, 0.f));
    const ImVec2 _display_size = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(_display_size);
    if (ImGui::Begin("Lucaria splash", nullptr, fullscreen_window_flags)) {
        draw_background(_display_size);
        draw_title(_display_size);
        draw_text(_display_size);
        ImGui::End();
    }
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(4);
}

static void add_next_level()
{
    make_scene<zone_flight_scene>();
    make_scene<user_player_scene>();
}

}

menu_splash_scene::menu_splash_scene(scene_data& scene)
{
    if (!detail::is_splash_resources_fetched) {
        detail::big_splash_font.emplace(fetch_font({ "assets/font/font_LYj3.bin" }, 160.f));
        detail::small_menu_font.emplace(fetch_font({ "assets/font/font_xVRp.bin" }, 22.f));
        detail::background_splash_texture.emplace(fetch_texture("assets/image/image_0cSX.bin"));
    }

    const entt::entity _splash_entity = scene.components.create();

    scene.components.emplace<widget_component>(_splash_entity)
        .gui([]() {
            if (detail::is_splash_resources_fetched) {
                static bool _has_added_first_level = false;
                if (!_has_added_first_level || (_has_added_first_level && get_fetches_waiting() > 0)) {
                    detail::draw_splash_menu();
                    detail::splash_resources_fetched_cursor += 1.f / 60.f; //get_time_delta();
                }
                if (get_is_audio_locked() && get_is_mouse_locked()) {
                    if (!_has_added_first_level) {
                        detail::add_next_level();
                        _has_added_first_level = true;
                    }
                }
            } else {
                if (detail::big_splash_font.has_value() && detail::small_menu_font.has_value() && detail::background_splash_texture.has_value()) {
                    detail::is_splash_resources_fetched = true;
                }
            }
        });
}