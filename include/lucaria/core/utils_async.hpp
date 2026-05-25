#pragma once

#include <functional>
#include <future>
#include <optional>
#include <type_traits>
#include <vector>

#include <lucaria/core/utils_error.hpp>

namespace lucaria {
namespace detail {

    template <typename FetchedType>
    struct container_async {

        container_async() = default;

        container_async(FetchedType&& value)
            : _cache(std::move(value))
            , _callbacks_invoked(true)
        {
        }

        container_async(std::future<FetchedType>&& future)
        {
            std::shared_ptr<std::future<FetchedType>> _shared_future = std::make_shared<std::future<FetchedType>>(std::move(future));

            _poll = [_shared_future]() -> bool {
                if (!_shared_future->valid()) {
                    return false;
                }
                return _shared_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
            };

            _get = [_shared_future]() -> FetchedType {
                return _shared_future->get();
            };
        }

        template <typename OriginFetchedType, typename ThenCallback, typename = std::enable_if_t<std::is_invocable_r_v<FetchedType, const ThenCallback&, const OriginFetchedType&>>>
        container_async(std::future<OriginFetchedType>&& future, const ThenCallback& then)
        {
            std::shared_ptr<std::future<OriginFetchedType>> _shared_intermediate_future = std::make_shared<std::future<OriginFetchedType>>(std::move(future));
            std::shared_ptr<std::decay_t<ThenCallback>> _shared_decayed_then = std::make_shared<std::decay_t<ThenCallback>>(then);

            _poll = [_shared_intermediate_future]() -> bool {
                if (!_shared_intermediate_future->valid()) {
                    return false;
                }
                return _shared_intermediate_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
            };

            _get = [_shared_intermediate_future, _shared_decayed_then]() -> FetchedType {
                const OriginFetchedType _intermediate_value = _shared_intermediate_future->get();
                return std::invoke(*_shared_decayed_then, _intermediate_value);
            };
        }

        [[nodiscard]] bool has_value() const
        {
            if (_cache) {
                _invoke_callbacks_once();
                return true;
            }

            if (_poll && _poll()) {
                _cache = std::move(_get());
                _poll = nullptr;
                _get = nullptr;
                _invoke_callbacks_once();
                return true;
            }

            return false;
        }

        [[nodiscard]] FetchedType& value()
        {
            if (!has_value()) {
                LUCARIA_DEBUG_ERROR("Failed to get fetched value&, please check has_value() before trying to access it")
            }

            return _cache.value();
        }

        [[nodiscard]] const FetchedType& value() const
        {
            if (!has_value()) {
                LUCARIA_DEBUG_ERROR("Failed to get fetched const value&, please check has_value() before trying to access it")
            }

            return _cache.value();
        }

        void on_ready(std::function<void(FetchedType&)> callback) const
        {
            if (has_value()) {
                callback(_cache.value());
                return;
            }

            _callbacks.emplace_back(std::move(callback));
        }

        void on_ready(std::function<void()> callback) const
        {
            on_ready([callback = std::move(callback)](FetchedType&) {
                callback();
            });
        }

        [[nodiscard]] explicit operator bool() const
        {
            return has_value();
        }

    private:
        mutable std::function<bool()> _poll = nullptr;
        mutable std::function<FetchedType()> _get = nullptr;
        mutable std::optional<FetchedType> _cache = std::nullopt;
        mutable bool _callbacks_invoked = false;
        mutable std::vector<std::function<void(FetchedType&)>> _callbacks = {};

        void _invoke_callbacks_once() const
        {
            if (_callbacks_invoked || !_cache) {
                return;
            }
            _callbacks_invoked = true;

            for (std::function<void(FetchedType&)>& _callback : _callbacks) {
                _callback(_cache.value());
            }
            _callbacks.clear();
        }
    };

}
}
