#pragma once

#include <core/fetch.hpp>
#include <core/font.hpp>
#include <core/texture.hpp>
#include <core/weight.hpp>
#include <core/world.hpp>

struct menu_splash_actor {
    menu_splash_actor(scene_data& scene);
    void update();

private:
    fetch_container<font_ref> big_splash_font = {};
    fetch_container<font_ref> small_menu_font = {};
    fetch_container<texture_ref> background_splash_texture = {};
    bool is_splash_resources_fetched = false;
    float splash_resources_fetched_cursor = 0.f;
    fadein_weight splash_background_fadein = { 3.f };
    oscillate_weight small_text_oscillate = { 2.f };
};