#include <glue/update.hpp>

void update()
{
    for (const std::function<void()>& _callback : detail::updaters) {
        _callback();
    }
}