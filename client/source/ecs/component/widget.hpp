#pragma once 

#include <core/font.hpp>

struct widget_component {
    widget_component() = default;
    widget_component(const widget_component& other) = delete;
    widget_component& operator=(const widget_component& other) = delete;
    widget_component(widget_component&& other) = default;
    widget_component& operator=(widget_component&& other) = default;

    widget_component& gui(const std::function<void()>& callback);

private:
    std::function<void()> _callback = nullptr;
    friend struct interface_system;
};