#pragma once

#include <lucaria/bin/types_math.hpp>

namespace lucaria {
namespace detail {

    struct game_cpu_cores {
        uint32 logical_threads = 1;
        uint32 physical_cores = 1;
        uint32 smt_threads_per_core = 1;
    };

    [[nodiscard]] game_cpu_cores detect_cpu_cores();

}
}
