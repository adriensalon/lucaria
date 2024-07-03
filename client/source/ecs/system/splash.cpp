#include <iostream>

#include <imgui.h>

#include <ecs/system/splash.hpp>
#include <glue/fetch.hpp>

namespace detail {

static bool is_setup = false;
static bool is_splash_done;
static std::chrono::system_clock::time_point start;
static std::optional<std::future<texture_data>> future_texture;
static std::optional<texture_ref> texture;
static std::chrono::seconds duration;

static void draw_splash()
{
    ImGui::SetNextWindowSize({ 500, 800 });
    if (ImGui::Begin("Lucaria splash")) {
        const std::string text = "Loading assets (" + std::to_string(get_fetches_completed()) + "/" + std::to_string(get_fetches_total()) + ")";
        ImGui::Text(text.c_str());
        if (texture.has_value())
            ImGui::Image((ImTextureID)(texture.value().get_id()), { 500, 500 });
        ImGui::End();
    }
}
}

void splash_system::splash_texture(std::future<texture_data>&& texture)
{
    detail::is_setup = true;
    detail::is_splash_done = false;
    detail::start = std::chrono::system_clock::now();
    detail::future_texture = std::move(texture);
    detail::texture = std::nullopt;
}

void splash_system::splash_duration(const std::chrono::seconds& duration)
{
    detail::is_setup = true;
    detail::is_splash_done = false;
    detail::start = std::chrono::system_clock::now();
    detail::duration = duration;
}

void splash_system::update()
{
#if LUCARIA_DEBUG
    if (!detail::is_setup) {
        std::cout << "Splash system is not setup" << std::endl;
    }
#endif
    // if (detail::is_splash_done) {
    //     return;
    // }
    if (!detail::texture.has_value() && get_is_future_ready(detail::future_texture.value())) {
        detail::texture = texture_ref(detail::future_texture.value().get());
    }
    const std::chrono::system_clock::duration _duration = std::chrono::system_clock::now() - detail::start;
    const std::size_t _fetches_completed = get_fetches_completed();
    const std::size_t _fetches_failed = get_fetches_failed();
    const std::size_t _fetches_total = get_fetches_total();
    const bool _all_fetches_ended = _fetches_completed + _fetches_failed == _fetches_total;
    const bool _duration_over = std::chrono::duration_cast<std::chrono::seconds>(_duration) > detail::duration;
    if (!_all_fetches_ended || !_duration_over) {
        detail::draw_splash();
        return;
    }
    detail::is_splash_done = true;
}
