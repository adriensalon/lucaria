#pragma once

#include <functional>
#include <future>
#include <type_traits>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/app_error.hpp>

namespace lucaria {
namespace detail {

    template <typename Asset>
    struct assets_async_slot {
        assets_async_slot() = default;

        assets_async_slot(Asset&& value)
            : _cache(std::move(value))
            , _cache_ready(true)
            , _callbacks_invoked(true)
        {
        }

        static assets_async_slot pending(Asset&& value = Asset {})
        {
            assets_async_slot _result = {};
            _result._cache = std::move(value);
            _result._cache_ready = false;
            _result._callbacks_invoked = false;
            return _result;
        }

        assets_async_slot(std::future<Asset>&& future)
        {
            std::shared_ptr<std::future<Asset>> _shared_future = std::make_shared<std::future<Asset>>(std::move(future));
            _poll = [_shared_future]() -> bool {
                if (!_shared_future->valid()) {
                    return false;
                }
                return _shared_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
            };
            _get = [_shared_future]() -> Asset {
                return _shared_future->get();
            };
        }

        template <typename OriginAsset, typename ThenCallback, typename = std::enable_if_t<std::is_invocable_r_v<Asset, const ThenCallback&, const OriginAsset&>>>
        assets_async_slot(std::future<OriginAsset>&& future, const ThenCallback& then)
        {
            std::shared_ptr<std::future<OriginAsset>> _shared_intermediate_future = std::make_shared<std::future<OriginAsset>>(std::move(future));
            std::shared_ptr<std::decay_t<ThenCallback>> _shared_decayed_then = std::make_shared<std::decay_t<ThenCallback>>(then);
            _poll = [_shared_intermediate_future]() -> bool {
                if (!_shared_intermediate_future->valid()) {
                    return false;
                }
                return _shared_intermediate_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
            };
            _get = [_shared_intermediate_future, _shared_decayed_then]() -> Asset {
                const OriginAsset _intermediate_value = _shared_intermediate_future->get();
                return std::invoke(*_shared_decayed_then, _intermediate_value);
            };
        }

        [[nodiscard]] bool has_value() const
        {
            if (_cache && _cache_ready) {
                _invoke_callbacks_once();
                return true;
            }
            if (_poll && _poll()) {
                _cache = std::move(_get());
                _cache_ready = true;
                _poll = nullptr;
                _get = nullptr;
                _invoke_callbacks_once();
                return true;
            }
            return false;
        }

        [[nodiscard]] bool has_emplaced_value() const
        {
            return _cache.has_value();
        }

        [[nodiscard]] Asset& emplaced_value()
        {
            LUCARIA_DEBUG_ASSERT(_cache, "Failed to get emplaced fetched value&, asset was not emplaced")
            return _cache.value();
        }

        [[nodiscard]] const Asset& emplaced_value() const
        {
            LUCARIA_DEBUG_ASSERT(_cache, "Failed to get emplaced fetched const value&, asset was not emplaced")
            return _cache.value();
        }

        void mark_ready() const
        {
            LUCARIA_DEBUG_ASSERT(_cache, "Failed to mark async container ready, asset was not emplaced")
            _cache_ready = true;
            _invoke_callbacks_once();
        }

        [[nodiscard]] Asset& value()
        {
            LUCARIA_DEBUG_ASSERT(has_value(), "Failed to get fetched value&, please check has_value() before trying to access it")
            return _cache.value();
        }

        [[nodiscard]] const Asset& value() const
        {
            LUCARIA_DEBUG_ASSERT(has_value(), "Failed to get fetched const value&, please check has_value() before trying to access it")
            return _cache.value();
        }

        void on_ready(std::function<void(Asset&)> callback) const
        {
            if (has_value()) {
                callback(_cache.value());
                return;
            }
            _callbacks.emplace_back(std::move(callback));
        }

        void on_ready(std::function<void()> callback) const
        {
            on_ready([callback = std::move(callback)](Asset&) {
                callback();
            });
        }

        [[nodiscard]] explicit operator bool() const
        {
            return has_value();
        }

    private:
        mutable std::function<bool()> _poll = nullptr;
        mutable std::function<Asset()> _get = nullptr;
        mutable std::optional<Asset> _cache = std::nullopt;
        mutable bool _cache_ready = false;
        mutable bool _callbacks_invoked = false;
        mutable std::vector<std::function<void(Asset&)>> _callbacks = {};

        void _invoke_callbacks_once() const
        {
            if (_callbacks_invoked || !_cache) {
                return;
            }
            _callbacks_invoked = true;
            for (std::function<void(Asset&)>& _callback : _callbacks) {
                _callback(_cache.value());
            }
            _callbacks.clear();
        }
    };

}
}
