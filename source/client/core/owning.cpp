#include <lucaria/core/error.hpp>
#include <lucaria/core/owning.hpp>

namespace lucaria {
namespace detail {

    owning_flag::owning_flag(owning_flag&& other) noexcept
        : _is_owning(std::exchange(other._is_owning, false))
    {
    }

    owning_flag& owning_flag::operator=(owning_flag&& other) noexcept
    {
        if (this != &other) {
            if (_is_owning) {
                LUCARIA_RUNTIME_ERROR("Object already owning resources")
            }

            _is_owning = std::exchange(other._is_owning, false);
        }

        return *this;
    }

    void owning_flag::emplace() noexcept
    {
        if (_is_owning) {
            LUCARIA_RUNTIME_ERROR("Object already owning resources")
        }

		_is_owning = true;
    }

    bool owning_flag::owns() const noexcept
    {
        return _is_owning;
    }

}
}