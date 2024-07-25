#pragma once 

using gui_callback = std::function<void(const std::unordered_map<glm::uint, fetch_container<font_ref>>)>;

struct widget_component {
    transform_component() = default;
    transform_component(const transform_component& other) = delete;
    transform_component& operator=(const transform_component& other) = delete;
    transform_component(transform_component&& other) = default;
    transform_component& operator=(transform_component&& other) = default;

    widget_component& gui(const gui_callback& callback);

private:
    gui_callback _callback = nullptr;
};