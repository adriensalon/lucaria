#pragma once

#include <core/world.hpp>

struct character_speach_actor {
    character_speach_actor(scene_data& scene, const std::vector<std::string>& lines);
    void update();

    void say(const std::size_t index);
    void quiet();

    std::size_t get_index();

private:
    std::vector<std::string> _lines = {};
    std::size_t _index = 0;
};