#include <iostream>

#include <imgui.h>

#include <ecs/system/splash.hpp>
#include <glue/fetch.hpp>
#include <glue/window.hpp>

namespace detail {

static bool is_splash_on = false;
static std::optional<std::future<texture_data>> future_texture;
static std::optional<texture_ref> texture;

static void update_texture_if_needed()
{
    if (!texture.has_value() && get_is_future_ready(future_texture.value())) {
        texture = texture_ref(future_texture.value().get());
    }
}

static bool is_fetching_complete()
{
    const std::size_t _fetches_completed = get_fetches_completed();
    const std::size_t _fetches_failed = get_fetches_failed();
    const std::size_t _fetches_total = get_fetches_total();
    return _fetches_completed + _fetches_failed == _fetches_total;
}

static void draw_splash()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGuiWindowFlags _window_flags = ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoBringToFrontOnFocus |
                                        ImGuiWindowFlags_NoNavFocus |
                                        ImGuiWindowFlags_NoBackground;
    if (ImGui::Begin("Lucaria splash", nullptr, _window_flags)) {
        const std::string text = "Loading assets (" + std::to_string(get_fetches_completed()) + "/" + std::to_string(get_fetches_total()) + ")";
        ImGui::Text(text.c_str());
        if (texture.has_value())
            ImGui::Image((ImTextureID)(texture.value().get_id()), { 500, 500 });
        ImGui::End();
    }
}

}

bool splash_system::is_splash_on()
{
    return detail::is_splash_on;
}

void splash_system::splash_texture(std::future<texture_data>&& texture)
{
    detail::future_texture = std::move(texture);
    detail::texture = std::nullopt;
}

void splash_system::trigger_splash(const bool titlescreen)
{
    detail::is_splash_on = true;
}

void splash_system::update()
{
    detail::update_texture_if_needed();
    if (detail::is_splash_on) {
        detail::draw_splash();
        if (detail::is_fetching_complete() && is_audio_locked()) {
            detail::is_splash_on = false;
            // start timer fade out
        }
    }
}
