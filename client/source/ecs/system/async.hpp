#pragma once

#include <glue/update.hpp>

/// @brief 
struct async_system {
    async_system() = delete;
    async_system(const async_system& other) = delete;
    async_system& operator=(const async_system& other) = delete;
    async_system(async_system&& other) = delete;
    async_system& operator=(async_system&& other) = delete;

    static void update();

private:
    REGISTER_FOR_UPDATE(async_system)
};