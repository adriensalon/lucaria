
#include <chrono>
#include <optional>
#include <iostream>
#include <unordered_map>

#include <imgui.h>

namespace detail {

std::optional<std::chrono::system_clock::time_point> start;
bool is_splash_done = false;
extern bool is_room_loaded;

void setup_splash()
{
    start = std::chrono::system_clock::now();
}

void draw_splash(const std::size_t loaded, const std::size_t total)
{
    if (ImGui::Begin("Lucaria splash")) {
        // if (loaded < total) {
            const std::string text = "Loading assets (" + std::to_string(loaded) + "/" + std::to_string(total) + ")";
            ImGui::Text(text.c_str());
        // } else {
            // ImGui::Text("Entering world");
        // }
        ImGui::End();
    }
}

void update_splash(const std::chrono::seconds& duration, const std::size_t loaded, const std::size_t total)
{
    if (is_splash_done) {
        return;
    }
    if (!start.has_value()) {
        setup_splash();
    }
    const std::chrono::system_clock::duration _duration = std::chrono::system_clock::now() - start.value();
    if (!is_room_loaded || std::chrono::duration_cast<std::chrono::seconds>(_duration) < duration) {
        draw_splash(loaded, total);
        return;
    }
    is_splash_done = true;
}

}