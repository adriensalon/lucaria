#pragma once

#include <sstream>
#include <functional>
#include <future>

/// @brief 
/// @tparam value_t 
/// @param future 
/// @return 
template <typename value_t>
bool is_future_ready(const std::future<value_t>& future)
{
    return future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}

/// @brief Alternative to preloading files
void fetch_file(const std::string& url, const std::function<void(std::istringstream&)>& callback);