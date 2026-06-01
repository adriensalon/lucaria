#pragma once

namespace lucaria {
namespace traits {

    template <typename ComponentType>
    struct component_compute_enable : std::false_type { };

    template <typename ComponentType>
    inline constexpr bool component_compute_enable_v = component_compute_enable<ComponentType>::value;

}
}

#define LUCARIA_ENABLE_COMPUTE_COMPONENT(ComponentType) \
    namespace lucaria::traits {                         \
        template <>                                     \
        struct component_compute_enable<ComponentType> \
            : std::true_type { };                       \
    }