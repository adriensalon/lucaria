#pragma once

#include <future>

#include <glue/update.hpp>

struct async_system {



    static void update();

private:
    REGISTER_FOR_UPDATE(async_system)
};