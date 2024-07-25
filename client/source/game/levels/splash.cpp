#include <chrono>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <entt/entt.hpp>
#include <imgui.h>
#include <imgui_internal.h>

#include <core/weight.hpp>
#include <core/window.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/controller.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/speaker.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/component/widget.hpp>
#include <ecs/system/interface.hpp>
#include <ecs/system/mixer.hpp>
#include <ecs/system/rendering.hpp>
#include <ecs/system/scripting.hpp>

#include <game/levels/levels.hpp>

namespace detail {

fetch_container<font_ref> big_splash_font = {};
fetch_container<font_ref> small_menu_font = {};
fetch_container<texture_ref> background_splash_texture = {};

const ImVec4 white_text_color = { 1.f, 1.f, 1.f, 0.98f };

constexpr ImGuiWindowFlags fullscreen_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

static bool is_splash_resources_fetched = false;
static bool is_next_level_fetched = false;
static glm::float32 splash_resources_fetched_timestamp = 0.f;
static fadein_weight splash_background_fadein = { 3.f };

static void draw_background(const ImVec2& display_size)
{
    const glm::float32 _weight = splash_background_fadein.compute_weight(splash_resources_fetched_timestamp);
    const ImVec4 _tint = { 1.f, 1.f, 1.f, _weight };
    ImGui::Image((ImTextureID)background_splash_texture.value().get_id(), display_size, { 0.001f, 0.001f }, { 0.999f, 0.999f }, _tint, { 1.f, 1.f, 1.f, 1.f });
    splash_resources_fetched_timestamp += get_time_delta();
    
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

static void draw_text(const ImVec2& display_size, const bool is_ready)
{
    const std::string _text = is_ready ? "Press any key to enter" : "Loading assets (" + std::to_string(get_fetches_completed()) + "/" + std::to_string(get_fetches_total()) + ")";
    ImGui::PushFont(small_menu_font.value().get_font());
    const ImVec2 _text_size = ImGui::CalcTextSize(_text.c_str());
    const ImVec2 _text_pos = (display_size - _text_size) / 2.f + ImVec2(0.f, 100.f);
    ImGui::SetCursorPos(_text_pos);
    ImGui::Text(_text.c_str());
    ImGui::PopFont();
}

static void draw_splash_menu(const bool is_ready)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_Text, white_text_color);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, white_text_color);
    const ImVec2 _display_size = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(_display_size);
    if (ImGui::Begin("Lucaria splash", nullptr, fullscreen_window_flags)) {
        draw_background(_display_size);
        draw_title(_display_size);
        draw_text(_display_size, is_ready);
        ImGui::End();
    }
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
}

static void add_next_level()
{
    add_level(levelID_persistent_player);
    add_level(levelID_blockout_test);
    // TODO
}

}

void level_menu_splash(entt::registry& registry)
{
    if (!detail::is_splash_resources_fetched) {
        detail::big_splash_font.emplace(fetch_font({ "assets/planet.bin" }, 160.f));
        detail::small_menu_font.emplace(fetch_font({ "assets/sfprolight.bin" }, 22.f));
        detail::background_splash_texture.emplace(fetch_texture("assets/splash.bin"));
    }

    const entt::entity _splash_entity = registry.create();

    registry.emplace<widget_component>(_splash_entity)
        .gui([]() {
            if (detail::is_splash_resources_fetched) {
                const bool _is_ready = get_fetches_completed() == get_fetches_total();
                bool _last_frame = false;
                if (_is_ready && get_is_audio_locked()) {
                    if (get_fetches_completed() > 3) {
                        mark_remove_level(levelID_menu_splash);
                        _last_frame = true;
                    } else if (!detail::is_next_level_fetched) {
                        detail::add_next_level();
                        detail::is_next_level_fetched = true;
                    }
                }
                if (!_last_frame) {
                    detail::draw_splash_menu(_is_ready);
                }
            } else {
                if (detail::big_splash_font.has_value() && detail::small_menu_font.has_value() && detail::background_splash_texture.has_value()) {
                    detail::is_splash_resources_fetched = true;
                }
            }
        });
}