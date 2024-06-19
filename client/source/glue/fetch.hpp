#pragma once

#include <sstream>
#include <functional>
#include <future>

/// @brief Checks if a future has already completed
/// @tparam value_t is the future type
/// @param future is the future to be checked
/// @return if the future has completed
template <typename value_t>
bool is_future_ready(const std::future<value_t>& future);

/// @brief Alternative to preloading files
void fetch_file(const std::string& url, const std::function<void(std::istringstream&)>& callback);

#include <glue/fetch.inl>