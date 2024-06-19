#pragma once

#include <glue/fetch.hpp>

template <typename value_t>
bool is_future_ready(const std::future<value_t>& future)
{
    return future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}