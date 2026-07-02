#include <stdlib.h>
#include <time.h>

void *timer_start(void)
{
    clock_t *_start = malloc(sizeof(clock_t));
    if (_start) {
        *_start = clock();
    }
    return _start;
}

double timer_time(void *timer)
{
    if (!timer) {
        return 1.0;
    }

    const clock_t _start = *((clock_t *)timer);
    const clock_t _now = clock();
    const double _elapsed = (double)(_now - _start) / CLOCKS_PER_SEC;
    return _elapsed > 0.0 ? _elapsed : 1.0;
}

void timer_clear(void *timer)
{
    free(timer);
}
