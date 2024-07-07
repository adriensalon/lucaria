#pragma once

#include <glue/update.hpp>

struct dynamics_system {
    dynamics_system() = delete;
    dynamics_system(const dynamics_system& other) = delete;
    dynamics_system& operator=(const dynamics_system& other) = delete;
    dynamics_system(dynamics_system&& other) = delete;
    dynamics_system& operator=(dynamics_system&& other) = delete;

    static void update();

private:
    REGISTER_FOR_UPDATE(dynamics_system)
};