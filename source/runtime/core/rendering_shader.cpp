#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/rendering_shader.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static void _load_data_from_bytes(data_shader& data, const std::vector<char>& bytes)
        {
            detail::assets_bytes_stream _stream(bytes);
#if defined(LUCARIA_JSON_ASSETS)
            cereal::JSONInputArchive _archive(_stream);
#else
            cereal::PortableBinaryInputArchive _archive(_stream);
#endif
            _archive(data);
        }

    }

    object_shader::object_shader(data_shader&& data)
    {
        this->data = std::move(data);
    }

    object_shader::object_shader(const std::vector<char>& bytes)
    {
        _load_data_from_bytes(data, bytes);
    }

}
}