#include <fstream>

#include <lucaria/entity/game.hpp>

namespace lucaria {
namespace detail {

    game_context& engine_context()
    {
        static game_context _engine_context = {};
        return _engine_context;
    }

}

void game_context::save_snapshot(const std::filesystem::path& path)
{
	std::ofstream _ofstream(path, std::ios::binary);
	cereal::PortableBinaryOutputArchive _archive(_ofstream);
	// objects TODO LATER

	std::unordered_map<std::string, detail::scene_type_implementation>& _scene_types = detail::engine_scene_types();
	for (detail::scene_implementation& _scene : detail::engine_scenes()) {
        // components TODO LATER

		detail::scene_type_implementation& _scene_type = _scene_types.at(_scene.type_id);
		_scene_type.binary_save(_scene, _archive);
    }
}

void game_context::load_snapshot(const std::filesystem::path& path)
{
	std::ifstream _ifstream(path, std::ios::binary);
	cereal::PortableBinaryInputArchive _archive(_ifstream);
	// objects TODO LATER
	
	std::unordered_map<std::string, detail::scene_type_implementation>& _scene_types = detail::engine_scene_types();
	for (detail::scene_implementation& _scene : detail::engine_scenes()) {
        // components TODO LATER
		
		detail::scene_type_implementation& _scene_type = _scene_types.at(_scene.type_id);
		_scene_type.binary_load(_scene, _archive);
    }
}


}