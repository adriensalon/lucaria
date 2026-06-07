#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/rendering_program.hpp>

namespace lucaria {
namespace detail {

    namespace {

        // assets_async_slot<object_program> _fetch_program_async(manager_assets& fetches, const std::filesystem::path& vertex_data_path, const std::filesystem::path& fragment_data_path)
        // {
        //     std::vector<std::filesystem::path> _shaders_paths = { vertex_data_path, fragment_data_path };
        //     std::shared_ptr<std::promise<std::pair<object_shader, object_shader>>> _shaders_promise = std::make_shared<std::promise<std::pair<object_shader, object_shader>>>();
        //     fetches.fetch_bytes(_shaders_paths, [_shaders_promise](const std::vector<std::vector<char>>& _data_bytes) {
		// 		std::pair<object_shader, object_shader> _shaders = {
		// 			object_shader { _data_bytes[0] },
		// 			object_shader { _data_bytes[1] }
		// 		};
        // 		_shaders_promise->set_value(std::move(_shaders)); }, true);

        //     return assets_async_slot<object_program>(_shaders_promise->get_future(), [](const std::pair<object_shader, object_shader>& _from) {
        //         return object_program(_from.first, _from.second);
        //     });
        // }

    }

}

}
