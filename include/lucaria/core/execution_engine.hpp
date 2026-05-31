#pragma once

#include <lucaria/core/execution_system.hpp>
#include <lucaria/core/storage_registry.hpp>

namespace lucaria {
namespace detail {

    template <auto SystemFunction, typename... ComponentTypes>
    void run_dispatch_compute_cpu_fallback(container_segment_registry_cpu& registry, type_list<ComponentTypes...>)
    {
        static_assert(
            sizeof...(ComponentTypes) > 0,
            "LGSL CPU fallback needs at least one component argument for now");

        auto view = registry.view<ComponentTypes...>();
        view.each(lgsl_cpu_fallback_invoker<SystemFunction> {});
    }

}
}
