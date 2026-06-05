#include <utility>

#include <lucaria/core/assets_buffer.hpp>

namespace lucaria {
namespace detail {

    flag_refcount::flag_refcount(flag_refcount_control* control) noexcept
        : _control(control)
    {
        _retain();
    }

    flag_refcount::flag_refcount(const flag_refcount& other) noexcept
        : _control(other._control)
    {
        _retain();
    }

    flag_refcount& flag_refcount::operator=(const flag_refcount& other) noexcept
    {
        if (this != &other) {
            _release();
            _control = other._control;
            _retain();
        }

        return *this;
    }

    flag_refcount::flag_refcount(flag_refcount&& other) noexcept
        : _control(std::exchange(other._control, nullptr))
    {
    }

    flag_refcount& flag_refcount::operator=(flag_refcount&& other) noexcept
    {
        if (this != &other) {
            _release();
            _control = std::exchange(other._control, nullptr);
        }

        return *this;
    }

    flag_refcount::~flag_refcount()
    {
        _release();
    }

    void flag_refcount::reset() noexcept
    {
        _release();
    }

    bool flag_refcount::owns() const noexcept
    {
        return _control != nullptr;
    }

    bool flag_refcount::is_last_owner() const noexcept
    {
        return _control && _control->count.load(std::memory_order_acquire) == 1;
    }

    uint32 flag_refcount::use_count() const noexcept
    {
        return _control
            ? _control->count.load(std::memory_order_acquire)
            : 0;
    }

    void flag_refcount::_retain() noexcept
    {
        if (_control) {
            _control->count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void flag_refcount::_release() noexcept
    {
        if (_control) {
            _control->count.fetch_sub(1, std::memory_order_acq_rel);
            _control = nullptr;
        }
    }

}
}