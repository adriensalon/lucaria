#pragma once

#include <core/world.hpp>

struct menu_splash_actor {
    menu_splash_actor(scene_data& scene);
    void update();

private:
    int _lol;
};