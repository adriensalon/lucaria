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
	// cereal::PortableBinaryOutputArchive _archive(_ofstream);
	cereal::JSONOutputArchive _archive(_ofstream);
	
	// objects
	detail::implementation_save_database _save_implementations = {};
	detail::recipe_save_database _save_recipes = detail::engine_resources().make_all_recipes(_save_implementations);
	_archive(cereal::make_nvp("objects", _save_recipes));
	
	// scenes
	std::unordered_map<std::string, detail::scene_type_implementation>& _scene_types = detail::engine_scene_types();
	for (detail::scene_implementation& _scene : detail::engine_scenes()) {
        // components TODO LATER

		detail::scene_type_implementation& _scene_type = _scene_types.at(_scene.type_id);
		_scene_type.json_save(_scene, _archive);
    }
}

void game_context::load_snapshot(const std::filesystem::path& path)
{
	std::ifstream _ifstream(path, std::ios::binary);
	// cereal::PortableBinaryInputArchive _archive(_ifstream);
	cereal::JSONInputArchive _archive(_ifstream);
	
	// objects
	detail::recipe_save_database _save_recipes = {};
	_archive(cereal::make_nvp("objects", _save_recipes));
	detail::object_save_database _save_objects = detail::engine_resources().apply_recipes(std::move(_save_recipes));
	
	// scenes
	std::unordered_map<std::string, detail::scene_type_implementation>& _scene_types = detail::engine_scene_types();
	for (detail::scene_implementation& _scene : detail::engine_scenes()) {
        // components TODO LATER
		
		detail::scene_type_implementation& _scene_type = _scene_types.at(_scene.type_id);
		// _scene_type.binary_load(_scene, _archive);
		_scene_type.json_load(_scene, _archive);
    }
}


}