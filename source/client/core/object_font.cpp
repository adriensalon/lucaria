#include <cstring>

#include <woff2/decode.h>

#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/manager_window.hpp>
#include <lucaria/core/object_font.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static ImFont* _load_font_bytes(manager_window& window, const std::vector<char>& data_bytes, const float32 font_size)
        {
            int _data_size = static_cast<int>(data_bytes.size());
            void* _owned_data = IM_ALLOC(_data_size);
            std::memcpy(_owned_data, data_bytes.data(), _data_size);
            ImFontConfig _config;
            _config.FontDataOwnedByAtlas = true;
            ImFont* _font = window.shared_font_atlas->AddFontFromMemoryTTF(_owned_data, _data_size, font_size, &_config);
            if (!window.shared_font_atlas->Build()) {
                LUCARIA_DEBUG_ERROR("Failed to build ImGui font atlas")
            }
            window.reupload_shared_imgui_font_texture();
            return _font;
        }

        static container_async<object_font> _fetch_font_async(manager_window& window, manager_object& objects, const std::filesystem::path& data_path, const float32 font_size)
        {
            std::shared_ptr<std::promise<std::shared_ptr<std::vector<char>>>> _data_promise = std::make_shared<std::promise<std::shared_ptr<std::vector<char>>>>();
            objects.fetch_bytes(data_path, [_data_promise](const std::vector<char>& _data_bytes) {
				const std::uint8_t* _raw_ptr = reinterpret_cast<const uint8_t*>(_data_bytes.data());
				const std::size_t _expected_size = woff2::ComputeWOFF2FinalSize(_raw_ptr, _data_bytes.size());
				if (_expected_size == 0) {
					LUCARIA_DEBUG_ERROR("Failed to compute woff2 final size");
				}
				std::string _output_str;
				_output_str.reserve(std::min(_expected_size, woff2::kDefaultMaxSize));
				woff2::WOFF2StringOut _woff2out(&_output_str);
				if (!woff2::ConvertWOFF2ToTTF(_raw_ptr, _data_bytes.size(), &_woff2out)) {
					LUCARIA_DEBUG_ERROR("Impossible to decode woff2 font")
				}
				std::shared_ptr<std::vector<char>> _shared_output = std::make_shared<std::vector<char>>();
				_shared_output->assign(std::make_move_iterator(_output_str.begin()), std::make_move_iterator(_output_str.end()));
				_data_promise->set_value(std::move(_shared_output)); }, true);

            // create font on main thread
            return container_async<object_font>(_data_promise->get_future(), [&window, font_size](const std::shared_ptr<std::vector<char>>& bytes) {
                return object_font(window, *bytes, font_size);
            });
        }

    }

    container_cache<object_font>& fetch(
        manager_window& window,
        manager_object& objects,
        container_cache_vector<object_font>& cached_vector,
        const std::filesystem::path& path,
        const float32 font_size)
    {
        return *cached_vector.get_or_create_by_path(path, [&window, &objects, path, font_size] {
            return _fetch_font_async(window, objects, path, font_size);
        });
    }

    object_font::object_font(manager_window& window, const std::vector<char>& bytes, const float32 font_size)
        : origin(object_font_origin::path)
    {
        font = _load_font_bytes(window, bytes, font_size);
    }

    recipe_object_font make_recipe(const container_cache<object_font>& cached)
    {
        const object_font& _font = cached.fetched.value();

        if (_font.origin == object_font_origin::path) {
            return recipe_object_font_path { cached.origin_path.value(), _font.font->FontSize };
        }

        else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

	container_cache<object_font>* apply_recipe(manager_window& window, manager_object& objects, container_cache_vector<object_font>& cached_vector, recipe_object_font& recipe)
    {
        return std::visit([&](auto& value) -> container_cache<object_font>* {
            using RecipeType = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<RecipeType, recipe_object_font_path>) {
                return &fetch(window, objects, cached_vector, value.path, value.font_size);

            } else {
                LUCARIA_DEBUG_ERROR("Implementation error");
				return nullptr;
            }
        },
            recipe);
    }

}
}
