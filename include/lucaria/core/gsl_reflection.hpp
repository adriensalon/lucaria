#pragma once

#include <array>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <vector>

#include <lucaria/engine/handle_entity.hpp>

namespace lucaria {

struct handle_entity;

namespace detail {

#if !defined(LUCARIA_DISABLE_COMPUTE)
    template <auto SystemFunction>
    struct gsl_system_stub;
#endif

    enum struct gsl_component_access_mode {
        read,
        write
    };

    struct gsl_component_info {
        const char* type_name;
        std::uint64_t type_id;
        gsl_component_access_mode access;
        bool is_entity;
    };

    struct gsl_system_info {
        const char* name = nullptr;
        const char* gsl_id = nullptr;
        const char* gsl_source = nullptr;
        const char* file = nullptr;
        int line = 0;
        void* function_ptr = nullptr;
        const gsl_component_info* parameters = nullptr;
        std::size_t parameter_count = 0;
    };

    template <typename>
    struct gsl_system_traits;

    template <typename ReturnType, typename... ArgTypes>
    struct gsl_system_traits<ReturnType (*)(ArgTypes...)> {
        using return_type = ReturnType;
        using arguments = std::tuple<ArgTypes...>;

        static constexpr std::size_t arity = sizeof...(ArgTypes);

        template <std::size_t ArgIndex>
        using argument = std::tuple_element_t<ArgIndex, std::tuple<ArgTypes...>>;
    };

    template <auto FunctionPtr>
    struct gsl_system_pointer_traits {
        using type = gsl_system_traits<decltype(FunctionPtr)>;
    };

    template <typename T>
    struct gsl_component_type {
        using raw_type = std::remove_cvref_t<T>;
        using no_ref_type = std::remove_reference_t<T>;
        using component_type = std::remove_cv_t<no_ref_type>;

        static constexpr bool is_entity = std::is_same_v<raw_type, handle_entity>;
        static constexpr bool is_component = std::is_lvalue_reference_v<T> && !is_entity;
        static constexpr bool is_mutable = std::is_lvalue_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>> && !is_entity;
        static constexpr bool is_readonly = std::is_lvalue_reference_v<T> && std::is_const_v<std::remove_reference_t<T>> && !is_entity;
        static constexpr gsl_component_access_mode access = is_mutable ? gsl_component_access_mode::write : gsl_component_access_mode::read;
    };

    template <typename T>
    [[nodiscard]] const char* gsl_type_name()
    {
        return typeid(std::remove_cvref_t<T>).name();
    }

    template <typename T>
    [[nodiscard]] std::uint64_t gsl_type_id()
    {
        return typeid(std::remove_cvref_t<T>).hash_code();
    }

    template <typename ArgType>
    [[nodiscard]] gsl_component_info make_execution_parameter_info()
    {
        using info = gsl_component_type<ArgType>;
        static_assert(info::is_entity || info::is_component, "LGSL system parameters must be handle_entity, T&, or const T&");
        return gsl_component_info {
            .type_name = gsl_type_name<typename info::component_type>(),
            .type_id = gsl_type_id<typename info::component_type>(),
            .access = info::access,
            .is_entity = info::is_entity
        };
    }

    template <typename FunctionTraits, std::size_t... Indices>
    [[nodiscard]] auto make_execution_parameter_array_impl(std::index_sequence<Indices...>)
    {
        return std::array<gsl_component_info, sizeof...(Indices)> {
            make_execution_parameter_info<typename FunctionTraits::template argument<Indices>>()...
        };
    }

    template <auto FunctionPtr>
    struct execution_system_metadata {
        using traits = typename gsl_system_pointer_traits<FunctionPtr>::type;

        static_assert(std::is_void_v<typename traits::return_type>, "LGSL systems should return void");
        static constexpr std::size_t parameter_count = traits::arity;

        [[nodiscard]] static const std::array<gsl_component_info, parameter_count>& parameters()
        {
            static const auto value = make_execution_parameter_array_impl<traits>(std::make_index_sequence<parameter_count> {});
            return value;
        }
    };

    template <typename... Types>
    struct gsl_type_list {
    };

    template <typename TypeList, typename NewType>
    struct gsl_type_list_push_back;

    template <typename... Types, typename NewType>
    struct gsl_type_list_push_back<gsl_type_list<Types...>, NewType> {
        using type = gsl_type_list<Types..., NewType>;
    };

    template <typename T>
    struct gsl_argument_is_entity {
        static constexpr bool value = std::is_same_v<std::remove_cvref_t<T>, handle_entity>;
    };

    template <typename T>
    struct gsl_argument_is_component {
        static constexpr bool value = std::is_lvalue_reference_v<T> && !gsl_argument_is_entity<T>::value;
    };

    template <typename T>
    struct gsl_normalized_component_argument {
        using raw_type = std::remove_cv_t<std::remove_reference_t<T>>;
        using type = std::conditional_t<std::is_const_v<std::remove_reference_t<T>>, const raw_type, raw_type>;
    };

    template <typename Accumulator, typename Arg>
    struct gsl_component_list_append {
        using type = Accumulator;
    };

    template <typename... Current, typename Arg>
    struct gsl_component_list_append<gsl_type_list<Current...>, Arg> {
        using type = std::conditional_t<
            gsl_argument_is_component<Arg>::value,
            gsl_type_list<Current..., typename gsl_normalized_component_argument<Arg>::type>,
            gsl_type_list<Current...>>;
    };

    template <typename Accumulator, typename... Args>
    struct gsl_component_list_from_args;

    template <typename Accumulator>
    struct gsl_component_list_from_args<Accumulator> {
        using type = Accumulator;
    };

    template <typename Accumulator, typename Head, typename... Tail>
    struct gsl_component_list_from_args<Accumulator, Head, Tail...> {
        using next = typename gsl_component_list_append<Accumulator, Head>::type;
        using type = typename gsl_component_list_from_args<next, Tail...>::type;
    };

    template <typename Tuple>
    struct gsl_component_list_from_tuple;

    template <typename... Args>
    struct gsl_component_list_from_tuple<std::tuple<Args...>> {
        using type = typename gsl_component_list_from_args<gsl_type_list<>, Args...>::type;
    };

    template <auto FunctionPtr>
    struct gsl_system_component_list {
        using traits = typename gsl_system_pointer_traits<FunctionPtr>::type;
        using type = typename gsl_component_list_from_tuple<typename traits::arguments>::type;
    };

    template <auto FunctionPtr>
    using lgsl_system_component_list_t = typename gsl_system_component_list<FunctionPtr>::type;

}
}