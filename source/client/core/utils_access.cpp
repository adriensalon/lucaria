#include <lucaria/core/utils_access.hpp>

#include <lucaria/core/manager_game.hpp>
#include <lucaria/core/manager_input.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/manager_window.hpp>
#include <lucaria/core/system_dynamics.hpp>
#include <lucaria/core/system_mixer.hpp>
#include <lucaria/core/system_motion.hpp>
#include <lucaria/core/system_rendering.hpp>
#include <lucaria/public/context_dynamics.hpp>
#include <lucaria/public/context_game.hpp>
#include <lucaria/public/context_input.hpp>
#include <lucaria/public/context_mixer.hpp>
#include <lucaria/public/context_object.hpp>
#include <lucaria/public/context_rendering.hpp>
#include <lucaria/public/context_scene.hpp>
#include <lucaria/public/context_window.hpp>

namespace lucaria {

void access_context::set(detail::manager_game& manager, context_game& context)
{
    context._manager = &manager;
}

void access_context::set(detail::manager_input& manager, context_input& context)
{
    context._manager = &manager;
}

void access_context::set(detail::manager_assets& manager, context_object& context)
{
    context._manager = &manager;
}

void access_context::set(detail::manager_scenes& manager, context_scene& context)
{
    context._manager = &manager;
}

void access_context::set(detail::manager_window& manager, context_window& context)
{
    context._manager = &manager;
}

void access_context::set(detail::system_dynamics& system, context_dynamics& context)
{
    context._system = &system;
}

void access_context::set(detail::system_mixer& system, context_mixer& context)
{
    context._system = &system;
}

void access_context::set(detail::system_rendering& system, context_rendering& context)
{
    context._system = &system;
}

}