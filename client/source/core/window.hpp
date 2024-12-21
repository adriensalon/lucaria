#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <string>
#include <unordered_map>

#include <core/world.hpp>

enum struct keyboard_key {
    z,
    q,
    s,
    d,
    k,
    i,
    p,
    o
};

template <typename scene_t>
void run(const std::function<void()>& update)
{
    detail::run_impl([] () {
        make_scene<scene_t>();
    }, update);
}

void graphics_assert();
void audio_assert();
void on_audio_locked(const std::function<void()>& callback);
glm::uint get_samplerate();
std::unordered_map<keyboard_key, bool>& get_keys();
std::unordered_map<glm::uint, bool>& get_buttons();
glm::vec2 get_screen_size();
glm::vec2 get_mouse_position();
glm::vec2& get_mouse_position_delta();
glm::float64 get_time_delta();
bool get_is_etc_supported();
bool get_is_s3tc_supported();
bool get_is_mouse_locked();
bool get_is_audio_locked();

namespace detail {
    void run_impl(const std::function<void()>& start, const std::function<void()>& update);
}