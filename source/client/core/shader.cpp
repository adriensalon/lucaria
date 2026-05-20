#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/shader.hpp>
#include <lucaria/core/stream.hpp>
#include <lucaria/core/fetch.hpp>

namespace lucaria {
namespace {

    static void load_data_from_bytes(shader_data& data, const std::vector<char>& bytes)
    {
        detail::bytes_stream _stream(bytes);
#if defined(LUCARIA_JSON_ASSETS)
        cereal::JSONInputArchive _archive(_stream);
#else
        cereal::PortableBinaryInputArchive _archive(_stream);
#endif
        _archive(data);
    }

}

shader::shader(shader_data&& data)
{
    this->data = std::move(data);
}

shader::shader(const std::vector<char>& bytes)
{
    load_data_from_bytes(data, bytes);
}

detail::async_container<shader> fetch_shader(const std::filesystem::path data_path)
{
    std::shared_ptr<std::promise<shader>> _promise = std::make_shared<std::promise<shader>>();
    detail::fetch_bytes(data_path, [_promise](const std::vector<char>& _bytes) {
        shader _shader(_bytes);
        _promise->set_value(std::move(_shader)); }, true);

    return detail::async_container<shader>(_promise->get_future());
}

}