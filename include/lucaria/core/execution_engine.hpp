#pragma once

#include <lucaria/core/gsl_reflection.hpp>
#include <lucaria/core/storage_registry.hpp>

namespace lucaria {
namespace detail {

    template <auto SystemFunction>
    struct gsl_fallback_invoker {
        template <typename... ComponentRefs>
        void operator()(handle_entity entity, ComponentRefs&&... components) const
        {
            using traits = typename gsl_system_pointer_traits<SystemFunction>::type;
            _invoke_implementation<traits>(std::make_index_sequence<traits::arity> {}, entity, std::forward<ComponentRefs>(components)...);
        }

    private:
        template <typename Traits, std::size_t... Indices, typename... ComponentRefs>
        static void _invoke_implementation(std::index_sequence<Indices...>, handle_entity entity, ComponentRefs&&... components)
        {
            auto _component_tuple = std::forward_as_tuple(std::forward<ComponentRefs>(components)...);
            std::invoke(SystemFunction, make_arg<typename Traits::template argument<Indices>>(entity, _component_tuple)...);
        }

        template <typename Arg, typename ComponentTuple>
        static decltype(auto) make_arg(handle_entity entity, ComponentTuple& components)
        {
            using raw_arg = std::remove_cvref_t<Arg>;
            if constexpr (std::is_same_v<raw_arg, handle_entity>) {
                return entity;
            } else {
                using wanted_type = std::remove_cv_t<std::remove_reference_t<Arg>>;
                return get_component_arg<Arg, wanted_type>(components, std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<ComponentTuple>>> {});
            }
        }

        template <typename Arg, typename WantedType, typename ComponentTuple, std::size_t... Indices>
        static decltype(auto) get_component_arg(ComponentTuple& components, std::index_sequence<Indices...>)
        {
            return get_component_arg_impl<Arg, WantedType, ComponentTuple, Indices...>(components);
        }

        template <typename Arg, typename WantedType, typename ComponentTuple, std::size_t Head, std::size_t... Tail>
        static decltype(auto) get_component_arg_impl(ComponentTuple& components)
        {
            using current_reference = decltype(std::get<Head>(components));
            using current_type = std::remove_cv_t<std::remove_reference_t<current_reference>>;
            if constexpr (std::is_same_v<current_type, WantedType>) {
                if constexpr (std::is_const_v<std::remove_reference_t<Arg>>) {
                    return static_cast<const WantedType&>(
                        std::get<Head>(components));
                } else {
                    return static_cast<WantedType&>(
                        std::get<Head>(components));
                }
            } else {
                static_assert(sizeof...(Tail) > 0, "LGSL system argument component was not found in fallback view");
                return get_component_arg_impl<Arg, WantedType, ComponentTuple, Tail...>(components);
            }
        }
    };

    template <auto SystemFunction, typename... ComponentTypes>
    void run_dispatch_compute_cpu_fallback(container_segment_registry_cpu& registry, gsl_type_list<ComponentTypes...>)
    {
        static_assert(sizeof...(ComponentTypes) > 0, "LGSL CPU fallback needs at least one component argument for now");
        auto _view = registry.view<ComponentTypes...>();
        _view.each(gsl_fallback_invoker<SystemFunction> {});
    }

}
}
