#pragma once 

#include <glue/update.hpp>

struct mixer_system {


    static void update();

private:
    REGISTER_FOR_UPDATE(mixer_system)
};