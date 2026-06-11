#include <lucaria/core/manager_game.hpp>
#include <lucaria/engine/context_game.hpp>

namespace lucaria {

void context_game::save_snapshot(const std::filesystem::path& path)
{
	_manager->save_snapshot(path);
}

void context_game::load_snapshot(const std::filesystem::path& path)
{
	_manager->load_snapshot(path);
}

}