#pragma once

#include <lucaria/core/gsl_reflection.hpp>
#include <lucaria/core/scenes_registry.hpp>
#include <lucaria/core/user_components.hpp>

namespace lucaria {
namespace detail {

    template <std::size_t ComponentCount>
    struct gsl_texture_dispatch_item {
        std::uint32_t entity = {};
        std::array<std::uint32_t, ComponentCount> component_indices = {};
    };

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

    template <typename... ComponentTypes>
    struct gsl_texture_dispatch_builder {
        using item_type = gsl_texture_dispatch_item<sizeof...(ComponentTypes)>;

        template <typename ComputeView>
        static std::vector<item_type> build(ComputeView& view)
        {
            std::vector<item_type> items = {};
            items.reserve(static_cast<std::size_t>(view.size()));
            view.each_segment([&](auto segment) {
                for (std::uint32_t i = 0; i < segment.count(); ++i) {
                    item_type item = {};
                    item.entity = static_cast<std::uint32_t>(segment.entity(i));
                    fill_indices<ComponentTypes...>(segment, i, item);
                    items.push_back(item);
                }
            });
            return items;
        }

    private:
        template <typename... Ts, typename Segment>
        static void fill_indices(Segment& segment, std::uint32_t i, item_type& item)
        {
            fill_indices_impl<Ts...>(
                segment,
                i,
                item,
                std::index_sequence_for<Ts...> {});
        }

        template <typename... Ts, typename Segment, std::size_t... Indices>
        static void fill_indices_impl(Segment& segment, std::uint32_t i, item_type& item, std::index_sequence<Indices...>)
        {
            (
                (item.component_indices[Indices] = segment.template index<std::remove_cvref_t<Ts>>(i)),
                ...);
        }
    };

    template <auto SystemFunction, typename... ComponentTypes>
    void run_dispatch_compute_cpu_fallback(storage_registry& registry, gsl_type_list<ComponentTypes...>)
    {
        static_assert(sizeof...(ComponentTypes) > 0, "LGSL CPU fallback needs at least one component argument for now");
        auto _view = registry.view<ComponentTypes...>();
        _view.each(gsl_fallback_invoker<SystemFunction> {});
    }

    struct gsl_opengl_texture_dispatch_context {
        const char* system_name = nullptr;
        const char* gsl_id = nullptr;
        const char* gsl_source = nullptr;
        std::uint32_t dispatch_count = {};
        std::uint32_t dispatch_width = {};
        std::uint32_t dispatch_height = {};
        GLuint dispatch_texture = {};
        GLuint framebuffer = {};
        GLuint program = {};
    };

    template <auto SystemFunction, typename... ComponentTypes>
    void run_dispatch_compute_opengl_texture(storage_registry& registry, gsl_type_list<ComponentTypes...>)
    {
        static_assert(sizeof...(ComponentTypes) > 0, "GSL OpenGL texture dispatch needs at least one component argument");
        static_assert((traits::component_compute_enable_v<std::remove_cvref_t<ComponentTypes>> && ...), "OpenGL texture dispatch requires compute-enabled components");
        auto _view = registry.view_compute<ComponentTypes...>();
        using builder_type = gsl_texture_dispatch_builder<std::remove_cvref_t<ComponentTypes>...>;
        auto _dispatch_items = builder_type::build(_view);
        gsl_opengl_texture_dispatch_context _context = {};
        _context.system_name = gsl_system_stub<SystemFunction>::name;
        _context.gsl_id = gsl_system_stub<SystemFunction>::gsl_id;
        _context.gsl_source = gsl_system_stub<SystemFunction>::gsl_source;

        // Upload dispatch item texture.
        // Bind input component textures.
        // Bind output framebuffer.
        // Draw fullscreen rect/triangle.
        // dispatch_opengl_texture_system<SystemFunction, ComponentTypes...>(registry, _view, _dispatch_items, _context);
    }

}
}
