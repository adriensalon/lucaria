
#include <chrono>
#include <optional>
#include <iostream>
#include <unordered_map>

#include <imgui.h>

#include <glue/fetch.hpp>

namespace detail {

std::optional<std::chrono::system_clock::time_point> start;
bool is_splash_done = false;
extern bool is_room_loaded;

void setup_splash()
{
    start = std::chrono::system_clock::now();
}

void draw_splash()
{
    if (ImGui::Begin("Lucaria splash")) {
        // if (loaded < total) {
            const std::string text = "Loading assets (" + std::to_string(get_fetches_completed()) + "/" + std::to_string(get_fetches_total()) + ")";
            ImGui::Text(text.c_str());
        // } else {
            // ImGui::Text("Entering world");
        // }
        ImGui::End();
    }
}

}

void update_splash(const std::chrono::seconds& duration)
{
    if (detail::is_splash_done) {
        return;
    }
    if (!detail::start.has_value()) {
        detail::setup_splash();
    }
    const std::chrono::system_clock::duration _duration = std::chrono::system_clock::now() - detail::start.value();
    if (!detail::is_room_loaded || std::chrono::duration_cast<std::chrono::seconds>(_duration) < duration) {
        detail::draw_splash();
        return;
    }
    detail::is_splash_done = true;
}