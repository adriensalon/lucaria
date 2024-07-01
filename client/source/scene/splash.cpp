
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

void draw_splash(const std::unordered_map<std::string, std::pair<int, int>>& infos)
{
    if (ImGui::Begin("Lucaria splash")) {

        for (const auto& ok : infos) {
            const auto text = "Loading " + ok.first + " (" + std::to_string(ok.second.first) + "/" + std::to_string(ok.second.second) + ")";
            ImGui::Text(text.c_str());
        }
        ImGui::End();
    }
}

void update_splash(const std::chrono::seconds& duration, const std::unordered_map<std::string, std::pair<int, int>>& infos)
{
    if (is_splash_done) {
        return;
    }
    if (!start.has_value()) {
        setup_splash();
    }
    const std::chrono::system_clock::duration _duration = std::chrono::system_clock::now() - start.value();
    if (!is_room_loaded || std::chrono::duration_cast<std::chrono::seconds>(_duration) < duration) {
        draw_splash(infos);
        return;
    }
    is_splash_done = true;
}

}