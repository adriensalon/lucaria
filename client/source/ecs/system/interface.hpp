#pragma once




struct interface_system {
    interface_system() = delete;
    interface_system(const interface_system& other) = delete;
    interface_system& operator=(const interface_system& other) = delete;
    interface_system(interface_system&& other) = delete;
    interface_system& operator=(interface_system&& other) = delete;

    static void use_font(const glm::uint name, const std::shared_future<std::shared_ptr<font_ref>>& fetched_font);

    static void collect_gui_widgets();
};