#pragma once

#include <array>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <vector>

#include <lucaria/core/storage_entity.hpp>

namespace lucaria {

struct handle_entity;

namespace detail {

    enum struct execution_access_mode {
        read,
        write
    };

    struct execution_parameter_info {
        const char* type_name;
        std::uint64_t type_id;
        execution_access_mode access;
        bool is_entity;
    };

    struct execution_system_info {
        const char* name;
        std::uint64_t stable_id;
        const char* file;
        int line;
        void* function_ptr;
        const execution_parameter_info* parameters;
        std::size_t parameter_count;
    };

    template <typename>
    struct function_traits;

    template <typename ReturnType, typename... ArgTypes>
    struct function_traits<ReturnType (*)(ArgTypes...)> {
        using return_type = ReturnType;
        using arguments = std::tuple<ArgTypes...>;

        static constexpr std::size_t arity = sizeof...(ArgTypes);

        template <std::size_t ArgIndex>
        using argument = std::tuple_element_t<ArgIndex, std::tuple<ArgTypes...>>;
    };

    template <auto FunctionPtr>
    struct function_pointer_traits {
        using type = function_traits<decltype(FunctionPtr)>;
    };

    template <typename T>
    struct execution_parameter_type {
        using raw_type = std::remove_cvref_t<T>;
        using no_ref_type = std::remove_reference_t<T>;
        using component_type = std::remove_cv_t<no_ref_type>;

        static constexpr bool is_entity = std::is_same_v<raw_type, handle_entity>;
        static constexpr bool is_component = std::is_lvalue_reference_v<T> && !is_entity;
        static constexpr bool is_mutable = std::is_lvalue_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>> && !is_entity;
        static constexpr bool is_readonly = std::is_lvalue_reference_v<T> && std::is_const_v<std::remove_reference_t<T>> && !is_entity;
        static constexpr execution_access_mode access = is_mutable ? execution_access_mode::write : execution_access_mode::read;
    };

    template <typename T>
    [[nodiscard]] const char* execution_type_name()
    {
        return typeid(std::remove_cvref_t<T>).name();
    }

    template <typename T>
    [[nodiscard]] std::uint64_t execution_type_id()
    {
        return typeid(std::remove_cvref_t<T>).hash_code();
    }

    template <typename ArgType>
    [[nodiscard]] execution_parameter_info make_execution_parameter_info()
    {
        using info = execution_parameter_type<ArgType>;

        static_assert(
            info::is_entity || info::is_component,
            "LGSL system parameters must be handle_entity, T&, or const T&");

        return execution_parameter_info {
            .type_name = execution_type_name<typename info::component_type>(),
            .type_id = execution_type_id<typename info::component_type>(),
            .access = info::access,
            .is_entity = info::is_entity
        };
    }

    template <typename FunctionTraits, std::size_t... Indices>
    [[nodiscard]] auto make_execution_parameter_array_impl(std::index_sequence<Indices...>)
    {
        return std::array<execution_parameter_info, sizeof...(Indices)> {
            make_execution_parameter_info<typename FunctionTraits::template argument<Indices>>()...
        };
    }

    template <auto FunctionPtr>
    struct execution_system_metadata {
        using traits = typename function_pointer_traits<FunctionPtr>::type;

        static_assert(
            std::is_void_v<typename traits::return_type>,
            "LGSL systems should return void");

        static constexpr std::size_t parameter_count = traits::arity;

        [[nodiscard]] static const std::array<execution_parameter_info, parameter_count>& parameters()
        {
            static const auto value = make_execution_parameter_array_impl<traits>(std::make_index_sequence<parameter_count> {});
            return value;
        }
    };

    [[nodiscard]] inline std::uint64_t make_stable_lgsl_system_id(const char* name)
    {
        std::uint64_t hash = 1469598103934665603ull;
        while (*name) {
            hash ^= static_cast<unsigned char>(*name++);
            hash *= 1099511628211ull;
        }
        return hash;
    }

    template <typename... Types>
    struct type_list {
    };

    template <typename TypeList, typename NewType>
    struct type_list_push_back;

    template <typename... Types, typename NewType>
    struct type_list_push_back<type_list<Types...>, NewType> {
        using type = type_list<Types..., NewType>;
    };

    template <typename T>
    struct lgsl_argument_is_entity {
        static constexpr bool value = std::is_same_v<std::remove_cvref_t<T>, handle_entity>;
    };

    template <typename T>
    struct lgsl_argument_is_component {
        static constexpr bool value = std::is_lvalue_reference_v<T> && !lgsl_argument_is_entity<T>::value;
    };

    template <typename T>
    struct lgsl_normalized_component_argument {
        using raw_type = std::remove_cv_t<std::remove_reference_t<T>>;

        using type = std::conditional_t<
            std::is_const_v<std::remove_reference_t<T>>,
            const raw_type,
            raw_type>;
    };

    template <typename Accumulator, typename Arg>
    struct lgsl_component_list_append {
        using type = Accumulator;
    };

    template <typename... Current, typename Arg>
    struct lgsl_component_list_append<type_list<Current...>, Arg> {
        using type = std::conditional_t<
            lgsl_argument_is_component<Arg>::value,
            type_list<Current..., typename lgsl_normalized_component_argument<Arg>::type>,
            type_list<Current...>>;
    };

    template <typename Accumulator, typename... Args>
    struct lgsl_component_list_from_args;

    template <typename Accumulator>
    struct lgsl_component_list_from_args<Accumulator> {
        using type = Accumulator;
    };

    template <typename Accumulator, typename Head, typename... Tail>
    struct lgsl_component_list_from_args<Accumulator, Head, Tail...> {
        using next = typename lgsl_component_list_append<Accumulator, Head>::type;
        using type = typename lgsl_component_list_from_args<next, Tail...>::type;
    };

    template <typename Tuple>
    struct lgsl_component_list_from_tuple;

    template <typename... Args>
    struct lgsl_component_list_from_tuple<std::tuple<Args...>> {
        using type = typename lgsl_component_list_from_args<type_list<>, Args...>::type;
    };

    template <auto FunctionPtr>
    struct lgsl_system_component_list {
        using traits = typename function_pointer_traits<FunctionPtr>::type;
        using type = typename lgsl_component_list_from_tuple<typename traits::arguments>::type;
    };

    template <auto FunctionPtr>
    using lgsl_system_component_list_t = typename lgsl_system_component_list<FunctionPtr>::type;

    template <auto SystemFunction>
    struct lgsl_cpu_fallback_invoker {
        template <typename... ComponentRefs>
        void operator()(handle_entity entity, ComponentRefs&&... components) const
        {
            using traits = typename function_pointer_traits<SystemFunction>::type;
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
}
}