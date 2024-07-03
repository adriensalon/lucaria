#pragma once 

#include <glue/update.hpp>

struct mixer_system {
    REGISTER_FOR_UPDATE(mixer_system)


    static void update();
};