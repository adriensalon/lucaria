#pragma once

namespace lucaria {
namespace detail {
    struct manager_game;
    struct manager_input;
    struct manager_object;
    struct manager_scene;
    struct manager_window;
    struct system_dynamics;
    struct system_mixer;
    struct system_motion;
    struct system_rendering;
}

struct context_game;
struct context_input;
struct context_object;
struct context_scene;
struct context_window;
struct context_dynamics;
struct context_mixer;
struct context_rendering;

struct access_context {
    access_context() = default;
    access_context(const access_context& other) = delete;
    access_context& operator=(const access_context& other) = delete;
    access_context(access_context&& other) = delete;
    access_context& operator=(access_context&& other) = delete;

    void set(detail::manager_game& manager, context_game& context);
    void set(detail::manager_input& manager, context_input& context);
    void set(detail::manager_object& manager, context_object& context);
    void set(detail::manager_scene& manager, context_scene& context);
    void set(detail::manager_window& manager, context_window& context);
    void set(detail::system_dynamics& system, context_dynamics& context);
    void set(detail::system_mixer& system, context_mixer& context);
    void set(detail::system_rendering& system, context_rendering& context);
};

}