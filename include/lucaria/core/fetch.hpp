#pragma once

#include <filesystem>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <vector>

namespace lucaria {

using fetch_callback = std::function<void(std::istringstream&)>;
using fetch_raw_callback = std::function<void(const std::vector<char>&)>;
using multiple_fetch_callback = std::function<void(std::size_t, std::size_t, std::istringstream&)>;
using multiple_fetch_raw_callback = std::function<void(std::size_t, std::size_t, const std::vector<char>&)>;

template <typename value_t>
struct fetch_container {
    fetch_container() = default;
    fetch_container(const fetch_container& other) = delete;
    fetch_container& operator=(const fetch_container& other) = delete;
    fetch_container(fetch_container&& other) = default;
    fetch_container& operator=(fetch_container&& other) = default;

    void emplace(const std::shared_ptr<value_t>& value);
    void emplace(const std::shared_future<std::shared_ptr<value_t>>& fetched, const std::function<void()>& callback = nullptr);
    bool has_value() const;
    value_t& value();
    const value_t& value() const;

private:
    std::optional<std::shared_future<std::shared_ptr<value_t>>> _fetched = std::nullopt;
    std::shared_ptr<value_t> _value = nullptr;
    std::function<void()> _callback = nullptr;
};

void fetch_file(const std::filesystem::path& file, const fetch_callback& callback, const bool persist = true);
void fetch_file(const std::filesystem::path& file, const fetch_raw_callback& callback, const bool persist = true);
void fetch_files(const std::vector<std::filesystem::path>& files, const multiple_fetch_callback& callback, const bool persist = true);
void fetch_files(const std::vector<std::filesystem::path>& files, const multiple_fetch_raw_callback& callback, const bool persist = true);

std::size_t get_fetches_waiting();

void wait_one_fetched_container();
void wait_fetched_containers();

}

#include <lucaria/core/fetch.inl>
