#include <cstring>

#include <woff2/decode.h>

#include <lucaria/core/database.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/font.hpp>
#include <lucaria/core/window.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static ImFont* _load_font_bytes(const std::vector<char>& data_bytes, const glm::float32 font_size)
        {
            int _data_size = static_cast<int>(data_bytes.size());
            void* _owned_data = IM_ALLOC(_data_size);
            std::memcpy(_owned_data, data_bytes.data(), _data_size);
            ImFontConfig _config;
            _config.FontDataOwnedByAtlas = true;
            ImFont* _font = engine_window().shared_font_atlas->AddFontFromMemoryTTF(_owned_data, _data_size, font_size, &_config);
            if (!engine_window().shared_font_atlas->Build()) {
                LUCARIA_RUNTIME_ERROR("Failed to build ImGui font atlas")
            }
            engine_window().reupload_shared_imgui_font_texture();
            return _font;
        }

        static async_container<font_implementation> _fetch_font_async(const std::filesystem::path& data_path, const glm::float32 font_size)
        {
            std::shared_ptr<std::promise<std::shared_ptr<std::vector<char>>>> _data_promise = std::make_shared<std::promise<std::shared_ptr<std::vector<char>>>>();
            fetch_bytes(data_path, [_data_promise](const std::vector<char>& _data_bytes) {
        const std::uint8_t* _raw_ptr = reinterpret_cast<const uint8_t*>(_data_bytes.data());
        const std::size_t _expected_size = woff2::ComputeWOFF2FinalSize(_raw_ptr, _data_bytes.size());
        if (_expected_size == 0) {
            LUCARIA_RUNTIME_ERROR("Failed to compute woff2 final size");
        }
        std::string _output_str;
        _output_str.reserve(std::min(_expected_size, woff2::kDefaultMaxSize));
        woff2::WOFF2StringOut _woff2out(&_output_str);
        if (!woff2::ConvertWOFF2ToTTF(_raw_ptr, _data_bytes.size(), &_woff2out)) {
            LUCARIA_RUNTIME_ERROR("Impossible to decode woff2 font")
        }
        std::shared_ptr<std::vector<char>> _shared_output = std::make_shared<std::vector<char>>();
        _shared_output->assign(std::make_move_iterator(_output_str.begin()), std::make_move_iterator(_output_str.end()));
        _data_promise->set_value(std::move(_shared_output)); }, true);

            // create font on main thread
            return async_container<font_implementation>(_data_promise->get_future(), [font_size](const std::shared_ptr<std::vector<char>>& bytes) {
                return font_implementation(*bytes, font_size);
            });
        }

    }

    font_implementation::font_implementation(const std::vector<char>& data_bytes, const glm::float32 font_size)
    {
        font = _load_font_bytes(data_bytes, font_size);
    }

}

font_object::~font_object()
{
	if (_refcount.is_last_owner()) {
		_manager->destroy_cell(_resource);
	}
}

font_object font_object::fetch(const std::filesystem::path& path, const float32 font_size)
{
    detail::resource_container<detail::font_implementation>* _resource = detail::engine_resources().fonts.get_or_create_by_path(path, [&] {
        return detail::_fetch_font_async(path, font_size);
    });

    return font_object { _resource };
}

bool font_object::has_value() const
{
    return _resource && _resource->is_ready();
}

font_object::operator bool() const
{
    return has_value();
}

ImFont* font_object::imgui_font() const
{
    if (!_resource) {
        return nullptr;
    }

    return _resource->get().font;
}

font_object::font_object(detail::resource_container<detail::font_implementation>* resource)
    : _resource(resource)
{
}

}
