
#include <ecs/system/immediate.hpp>

void immediate_system::use_gui_mvp(const std::optional<glm::mat4>& mvp)
{
    gui_mvp(mvp);
}

[[nodiscard]] ImDrawList* immediate_system::get_gui_drawlist()
{
    return ImGui::GetBackgroundDrawList();
}

[[nodiscard]] std::unordered_map<keyboard_key, bool>& immediate_system::get_keys()
{
    return ::get_keys();
}

[[nodiscard]] std::unordered_map<glm::uint, bool>& immediate_system::get_buttons()
{
    return ::get_buttons();
}

[[nodiscard]] glm::vec2 immediate_system::get_screen_size()
{
    return ::get_screen_size();
}

[[nodiscard]] glm::vec2 immediate_system::get_mouse_position()
{
    return ::get_mouse_position();
}

[[nodiscard]] glm::vec2& immediate_system::get_mouse_position_delta()
{
    return ::get_mouse_position_delta();
}

[[nodiscard]] glm::float64 immediate_system::get_time_delta()
{
    return ::get_time_delta();
}
