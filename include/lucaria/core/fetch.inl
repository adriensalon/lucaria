#pragma once

#include <unordered_map>

namespace lucaria {
namespace detail {

    inline std::unordered_map<std::uintptr_t, std::function<bool()>> fetch_container_updaters = {};

    template <typename value_t>
    bool get_is_future_ready(const std::shared_future<value_t>& future)
    {
        return future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
    }

}

template <typename value_t>
void fetch_container<value_t>::emplace(const std::shared_ptr<value_t>& value)
{
#if LUCARIA_DEBUG
    if (_fetched) {
        std::cout << "Fetched container is already waiting for a future." << std::endl;
        std::terminate();
    }
#endif
    _value = value;
}

template <typename value_t>
void fetch_container<value_t>::emplace(const std::shared_future<std::shared_ptr<value_t>>& fetched, const std::function<void()>& callback)
{
#if LUCARIA_DEBUG
    if (_fetched) {
        std::cout << "Fetched container is already waiting for a future." << std::endl;
        std::terminate();
    }
#endif
    _fetched = fetched;
    _callback = callback;
    detail::fetch_container_updaters[reinterpret_cast<std::uintptr_t>(static_cast<void*>(this))] = [this]() {
        if (_fetched.has_value()) {
            std::shared_future<std::shared_ptr<value_t>>& _future_value = _fetched.value();
            if (detail::get_is_future_ready<std::shared_ptr<value_t>>(_future_value)) {
                _value = _future_value.get();
                if (_callback) {
                    _callback();
                }
                _fetched = std::nullopt;
                return true;
            }
        }
        return false;
    };
}

template <typename value_t>
bool fetch_container<value_t>::has_value() const
{
    return _value.operator bool();
}

template <typename value_t>
value_t& fetch_container<value_t>::value()
{
    return *(_value.get());
}

template <typename value_t>
const value_t& fetch_container<value_t>::value() const
{
    return *(_value.get());
}

}
