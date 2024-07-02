#pragma once

#include <sstream>
#include <filesystem>
#include <functional>
#include <future>
#include <vector>

/// @brief Alternative to preloading files
void fetch_file(const std::string& url, const std::function<void(std::istringstream&)>& callback, const bool persist = true);

/// @brief Alternative to preloading files
void fetch_files(const std::vector<std::filesystem::path>& files, const std::function<void(const std::size_t, const std::size_t, std::istringstream&)>& callback, const bool persist = true);

/// @brief 
/// @return
std::size_t get_fetches_completed();

/// @brief 
/// @return
std::size_t get_fetches_failed();

/// @brief 
/// @return 
std::size_t get_fetches_total();

/// @brief Checks if a future has already completed
/// @tparam value_t is the future type
/// @param future is the future to be checked
/// @return if the future has completed
template <typename value_t>
bool get_is_future_ready(const std::future<value_t>& future)
{
    return future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}